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

#include "moduleDistortion.h"
#include "globals.h"
#include <cmath>
#include <QDebug>


EffectModuleDistortion::EffectModuleDistortion(QObject * parent) : QObject(parent)
{
	setModuleName("EffectModuleDistortion");
}

EffectModuleDistortion::~EffectModuleDistortion()
{
	
}

QList<EffectBase::ParameterCluster> EffectModuleDistortion::getListOfParameterClusters()
{
	QList<EffectBase::ParameterCluster> cluster;
	
	QStringList list = QStringList();
	list.append(QString(tr("Radial")));
	list.append(QString(tr("Horizontal")));
	list.append(QString(tr("Vertical")));
	cluster += uiParamCombobox(tr("Direction"), "Direction", 0, list);

	cluster += uiParamSlider100(tr("Main coefficient"), "MainCoefficient", 0.0, -1.0, 1.0);
	cluster += uiParamSlider100(tr("Auxiliary coefficient"), "AuxCoefficient", 0.0, -1.0, 1.0);
	
	return cluster;
}

QRgb EffectModuleDistortion::interpolatedPixel(const QRgb ** rows, int w, int h, float x, float y)
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
	int r1, g1, b1, a1;
	int r2, g2, b2, a2;
	pixel1 = row1[iX];
	pixel2 = row1[iXp];
	r1 = (256 - fracX)*qRed(pixel1) + fracX*qRed(pixel2);
	g1 = (256 - fracX)*qGreen(pixel1) + fracX*qGreen(pixel2);
	b1 = (256 - fracX)*qBlue(pixel1) + fracX*qBlue(pixel2);
	a1 = (256 - fracX)*qAlpha(pixel1) + fracX*qAlpha(pixel2);
	pixel1 = row2[iX];
	pixel2 = row2[iXp];
	r2 = (256 - fracX)*qRed(pixel1) + fracX*qRed(pixel2);
	g2 = (256 - fracX)*qGreen(pixel1) + fracX*qGreen(pixel2);
	b2 = (256 - fracX)*qBlue(pixel1) + fracX*qBlue(pixel2);
	a2 = (256 - fracX)*qAlpha(pixel1) + fracX*qAlpha(pixel2);
	r1 = (256 - fracY)*r1 + fracY*r2;
	g1 = (256 - fracY)*g1 + fracY*g2;
	b1 = (256 - fracY)*b1 + fracY*b2;
	a1 = (256 - fracY)*a1 + fracY*a2;
	r1 /= 65536;
	g1 /= 65536;
	b1 /= 65536;
	a1 /= 65536;
	return qRgba(r1, g1, b1, a1); 
}

QImage EffectModuleDistortion::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	int direction = getParamIntValue(parameters, "Direction", 0);
	float c1 = getParamDoubleValue(parameters, "MainCoefficient", 0.0) / 10.0;
	float c2 = getParamDoubleValue(parameters, "AuxCoefficient", 0.0) / 10.0;
	
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
			float scale = 1.0f / (1.0f + c1*r2 + c2*r2*r2);
			px = centerX + px * scale;
			py = centerY + py * scale;
			dstRow[x] = interpolatedPixel(srcRows, w, h, px, py);
		}
	}

	dst = dst.convertToFormat(image.format());

	delete [] srcRows;
	return dst;
}

