// Copyright (C) 2026 zhuvoy
// 
// This file is part of ClassicImageViewer.
// 
// ClassicImageViewer is free software: you can redistribute it and/or modify it under the terms of the
// GNU General Public License as published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
// 
// ClassicImageViewer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with ClassicImageViewer.
// If not, see <https://www.gnu.org/licenses/>.

#include "moduleMotionBlur.h"
#include "globals.h"
#include <cmath>
#include <QDebug>


EffectModuleMotionBlur::EffectModuleMotionBlur(QObject * parent) : QObject(parent)
{
	setModuleName("EffectModuleMotionBlur");
}

EffectModuleMotionBlur::~EffectModuleMotionBlur()
{
	
}

QList<EffectBase::ParameterCluster> EffectModuleMotionBlur::getListOfParameterClusters()
{
	QList<EffectBase::ParameterCluster> cluster;
	
	QStringList list = QStringList();
	list.append(QString(tr("Linear")));
	list.append(QString(tr("Rotation")));
	list.append(QString(tr("Zoom")));
	cluster += uiParamCombobox(tr("Method"), "Method", 0, list);
	
	cluster += uiParamSlider(tr("Steps"), "Steps", 32, 2, 100);
	cluster += uiParamSlider(tr("Displacement"), "Displacement", 32, 1, 360);
	cluster += uiParamSlider(tr("Direction"), "Direction", 0, 0, 360);
	
	return cluster;
}

EffectModuleMotionBlur::intRGBA_t EffectModuleMotionBlur::interpolatedPixel(const QRgb ** rows, int w, int h, float x, float y)
{
	x = qBound(0.0f, x, static_cast<float>(w - 1));
	y = qBound(0.0f, y, static_cast<float>(h - 1));
	
	int iX, iY, iXp, iYp, fracX, fracY;
	iX = std::round(x * 256.0f);
	iY = std::round(y * 256.0f);
	fracX = iX % 256;
	fracY = iY % 256;
	iX /= 256;
	iY /= 256;
	
	iXp = fracX ? (iX + 1) : iX;
	iYp = fracY ? (iY + 1) : iY;
	
	const QRgb * row1 = rows[iY];
	const QRgb * row2 = rows[iYp];
	QRgb pixel1, pixel2;
	intRGBA_t rowRGBA1, rowRGBA2;
	pixel1 = row1[iX];
	pixel2 = row1[iXp];
	rowRGBA1.r = (256 - fracX)*qRed(pixel1) + fracX*qRed(pixel2);
	rowRGBA1.g = (256 - fracX)*qGreen(pixel1) + fracX*qGreen(pixel2);
	rowRGBA1.b = (256 - fracX)*qBlue(pixel1) + fracX*qBlue(pixel2);
	rowRGBA1.a = (256 - fracX)*qAlpha(pixel1) + fracX*qAlpha(pixel2);
	pixel1 = row2[iX];
	pixel2 = row2[iXp];
	rowRGBA2.r = (256 - fracX)*qRed(pixel1) + fracX*qRed(pixel2);
	rowRGBA2.g = (256 - fracX)*qGreen(pixel1) + fracX*qGreen(pixel2);
	rowRGBA2.b = (256 - fracX)*qBlue(pixel1) + fracX*qBlue(pixel2);
	rowRGBA2.a = (256 - fracX)*qAlpha(pixel1) + fracX*qAlpha(pixel2);
	rowRGBA1.r = (256 - fracY)*rowRGBA1.r + fracY*rowRGBA2.r;
	rowRGBA1.g = (256 - fracY)*rowRGBA1.g + fracY*rowRGBA2.g;
	rowRGBA1.b = (256 - fracY)*rowRGBA1.b + fracY*rowRGBA2.b;
	rowRGBA1.a = (256 - fracY)*rowRGBA1.a + fracY*rowRGBA2.a;
	rowRGBA1 /= 65536;
	return rowRGBA1;
}

QImage EffectModuleMotionBlur::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	int method = getParamIntValue(parameters, "Method", 0);
	int steps = getParamIntValue(parameters, "Steps", 32);
	int displacement = getParamIntValue(parameters, "Displacement", 32);
	int direction = getParamIntValue(parameters, "Direction", 0);
	
	bool hasAlpha = image.hasAlphaChannel();
	QImage src,dst;

	// convert to xRGB32 format if needed
	if (hasAlpha)
	{
		src = image.convertToFormat(QImage::Format_ARGB32).copy();
		dst = src.copy();
	}
	else
	{
		src = image.convertToFormat(QImage::Format_RGB32).copy();
		dst = src.copy();
	}

	auto srcRows = new const QRgb *[image.height()];
	for (int y = 0; y < image.height(); y++)
	{
		srcRows[y] = reinterpret_cast<const QRgb *>(src.constScanLine(y));
	}
	
	int w = image.width();
	int h = image.height();
	
	float linearDeltaX = (std::cos(direction * (M_PI/180.0f)) * displacement) / steps;
	float linearDeltaY = (std::sin(direction * (M_PI/180.0f)) * displacement) / steps;
	
	float middleX = w / 2.0f;
	float middleY = h / 2.0f;
	
	float * rotC = new float[steps];
	float * rotS = new float[steps];
	for (int i = 0; i < steps; i++)
	{
		float angle = 0.25 * displacement * M_PI/180.0f * i / steps;
		if (i%2) angle *= -1;
		rotC[i] = std::cos(angle);
		rotS[i] = std::sin(angle);
	}
	
	float halfDiagonal = std::sqrt(middleX*middleX + middleY*middleY);
	float zoomScale = std::pow(halfDiagonal / (halfDiagonal + displacement), 1.0 / steps);
	

	#pragma omp parallel for schedule(dynamic, 1)
	for (int y = 0; y < h; y++)
	{
		QRgb* dstRow = reinterpret_cast<QRgb *>(dst.scanLine(y)); 
		for (int x = 0; x < w; x++)
		{
			intRGBA_t pixel;
			int pixelCount = 0;
			switch (method)
			{
				default:
				case 0:
					{
						float px = x;
						float py = y;
						for (int i = 0; i < steps; i++)
						{
							//if ((py >= 0.0f) && (py < h) && (px >= 0.0f) && (px < w))
							{
								pixel += interpolatedPixel(srcRows, w, h, px, py);
								pixelCount++;
							}
							px += linearDeltaX;
							py += linearDeltaY;
						}
					}
					break;
				case 1:
					{
						float dx = x - middleX;
						float dy = y - middleY;
						for (int i = 0; i < steps; i++)
						{
							float px = middleX + rotC[i]*dx + rotS[i]*dy;
							float py = middleY + rotC[i]*dy - rotS[i]*dx;
							//if ((py >= 0.0f) && (py < h) && (px >= 0.0f) && (px < w))
							{
								pixel += interpolatedPixel(srcRows, w, h, px, py);
								pixelCount++;
							}
						}
					}
					break;
				case 2:
					{
						float dx = x - middleX;
						float dy = y - middleY;
						float px = x;
						float py = y;
						for (int i = 0; i < steps; i++)
						{
							//if ((py >= 0.0f) && (py < h) && (px >= 0.0f) && (px < w))
							{
								pixel += interpolatedPixel(srcRows, w, h, px, py);
								pixelCount++;
							}
							px = middleX + dx;
							py = middleY + dy;
							dx *= zoomScale;
							dy *= zoomScale;
						}
					}
					break;
			}
			pixel /= pixelCount;
			dstRow[x] = qRgba(pixel.r, pixel.g, pixel.b, pixel.a);
		}
	}
	
	dst = dst.convertToFormat(image.format());
	
	delete [] rotC;
	delete [] rotS;
	delete [] srcRows;
	
	return dst;
}

