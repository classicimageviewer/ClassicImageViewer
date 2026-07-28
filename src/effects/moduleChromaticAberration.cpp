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

#include "moduleChromaticAberration.h"
#include "globals.h"
#include <cmath>
#include <QDebug>


EffectModuleChromaticAberration::EffectModuleChromaticAberration(QObject * parent) : QObject(parent)
{
	setModuleName("EffectModuleChromaticAberration");
}

EffectModuleChromaticAberration::~EffectModuleChromaticAberration()
{
	
}

QList<EffectBase::ParameterCluster> EffectModuleChromaticAberration::getListOfParameterClusters()
{
	QList<EffectBase::ParameterCluster> cluster;
	
	QStringList list = QStringList();
	list.append(QString(tr("Radial")));
	list.append(QString(tr("Horizontal")));
	list.append(QString(tr("Vertical")));
	cluster += uiParamCombobox(tr("Direction"), "Direction", 0, list);

	cluster += uiParamSlider100(tr("Red"), "Red", 0.0, -1.0, 1.0);
	cluster += uiParamSlider100(tr("Blue"), "Blue", 0.0, -1.0, 1.0);
	
	return cluster;
}

int EffectModuleChromaticAberration::interpolatedPixelR(const QRgb ** rows, int w, int h, float x, float y)
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
	int r1;
	int r2;
	pixel1 = row1[iX];
	pixel2 = row1[iXp];
	r1 = (256 - fracX)*qRed(pixel1) + fracX*qRed(pixel2);
	pixel1 = row2[iX];
	pixel2 = row2[iXp];
	r2 = (256 - fracX)*qRed(pixel1) + fracX*qRed(pixel2);
	r1 = (256 - fracY)*r1 + fracY*r2;
	r1 /= 65536;
	return r1; 
}

int EffectModuleChromaticAberration::interpolatedPixelB(const QRgb ** rows, int w, int h, float x, float y)
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
	int b1;
	int b2;
	pixel1 = row1[iX];
	pixel2 = row1[iXp];
	b1 = (256 - fracX)*qBlue(pixel1) + fracX*qBlue(pixel2);
	pixel1 = row2[iX];
	pixel2 = row2[iXp];
	b2 = (256 - fracX)*qBlue(pixel1) + fracX*qBlue(pixel2);
	b1 = (256 - fracY)*b1 + fracY*b2;
	b1 /= 65536;
	return b1; 
}

QImage EffectModuleChromaticAberration::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	int direction = getParamIntValue(parameters, "Direction", 0);
	float red = getParamDoubleValue(parameters, "Red", 0.0) / 10.0;
	float blue = getParamDoubleValue(parameters, "Blue", 0.0) / 10.0;
	
	bool hasAlpha = image.hasAlphaChannel();
	QImage src,dst;

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

	int w = image.width();
	int h = image.height();
	float centerX = w / 2.0f;
	float centerY = h / 2.0f;
	float rNorm = qMax(centerX, centerY);
	float rNorm2Inv = 1.0f / (rNorm * rNorm);
	
	auto srcRows = new const QRgb *[h];
	for (int y = 0; y < h; y++)
	{
		srcRows[y] = reinterpret_cast<const QRgb *>(src.constScanLine(y));
	}
	
	#pragma omp parallel for schedule(dynamic, 1)
	for (int y = 0; y < h; y++)
	{
		QRgb* srcRow = reinterpret_cast<QRgb *>(src.scanLine(y));
		QRgb* dstRow = reinterpret_cast<QRgb *>(dst.scanLine(y)); 
		for (int x = 0; x < w; x++)
		{
			float px, py;
			px = x - centerX;
			py = y - centerY;
			float r2;
			switch(direction)
			{
				default:
				case 0:
					r2 = px*px + py*py;
					break;
				case 1:
					r2 = px*px;
					break;
				case 2:
					r2 = py*py;
					break;
			}
			r2 *= rNorm2Inv;
			float scaleR = 1.0f / (1.0f + red*r2);
			float scaleB = 1.0f / (1.0f + blue*r2);
			float pxR = centerX + px * scaleR;
			float pyR = centerY + py * scaleR;
			float pxB = centerX + px * scaleB;
			float pyB = centerY + py * scaleB;
			QRgb srcPixel = srcRow[x];
			dstRow[x] = qRgba(interpolatedPixelR(srcRows, w, h, pxR, pyR), qGreen(srcPixel), interpolatedPixelB(srcRows, w, h, pxB, pyB), qAlpha(srcPixel));
		}
	}

	dst = dst.convertToFormat(image.format());

	delete [] srcRows;
	return dst;
}

