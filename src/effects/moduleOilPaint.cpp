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

#include "moduleOilPaint.h"
#include "globals.h"
#include <omp.h>
#include <QDebug>


EffectModuleOilPaint::EffectModuleOilPaint(QObject * parent) : QObject(parent)
{
	setModuleName("EffectModuleOilPaint");
}

EffectModuleOilPaint::~EffectModuleOilPaint()
{
	
}

QList<EffectBase::ParameterCluster> EffectModuleOilPaint::getListOfParameterClusters()
{
	QList<EffectBase::ParameterCluster> cluster;
	
	cluster += uiParamSlider(tr("Radius"), "Radius", 10, 1, 25);
	cluster += uiParamSlider(tr("Levels"), "Levels", 20, 2, 255);
	
	return cluster;
}

QImage EffectModuleOilPaint::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	int radius = getParamIntValue(parameters, "Radius", 10);
	int levels = getParamIntValue(parameters, "Levels", 20);
	int radius2 = radius*radius;
	
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
	
	
	auto srcRows = new const QRgb *[image.height()];
	for (int y = 0; y < image.height(); y++)
	{
		srcRows[y] = reinterpret_cast<const QRgb *>(src.constScanLine(y));
	}
	
	int threadCount = omp_get_max_threads();
	if (threadCount < 1) threadCount = 1;
	
	int * scratch = new int [4*levels * threadCount];

	#pragma omp parallel for schedule(dynamic, 1)
	for (int y = 0; y < image.height(); y++)
	{
		int threadNum = omp_get_thread_num();
		assert(threadNum < threadCount);
		QRgb * dstRow = reinterpret_cast<QRgb *>(dst.scanLine(y));
		int * intensityCount = scratch + threadNum*4*levels + 0;
		int * averageR = scratch + threadNum*4*levels + 1*levels;
		int * averageG = scratch + threadNum*4*levels + 2*levels;
		int * averageB = scratch + threadNum*4*levels + 3*levels;
		for (int x = 0; x < image.width(); x++)
		{
			for (int i = 0; i < levels; i++)
			{
				intensityCount[i] = 0;
				averageR[i] = 0;
				averageG[i] = 0;
				averageB[i] = 0;
			}
			for (int ry = y - radius, dy = -radius; ry <= y + radius; ry++, dy++)
			{
				if (ry < 0) continue;
				if (ry >= image.height()) continue;
				for (int rx = x - radius, dx = -radius; rx <= x + radius; rx++, dx++)
				{
					if (rx < 0) continue;
					if (rx >= image.width()) continue;
					
					int r2 = dx*dx + dy+dy;
					if (r2 > radius2) continue;
					
					QRgb srcPixel = srcRows[ry][rx];
					
					int r = qRed(srcPixel);
					int g = qGreen(srcPixel);
					int b = qBlue(srcPixel);
					int intensity = (r + g + b) * levels / 765;
					if (intensity >= levels) intensity = levels - 1;
					
					intensityCount[intensity] += 1;
					averageR[intensity] += r;
					averageG[intensity] += g;
					averageB[intensity] += b;
				}
			}
			
			int maxIntensity = 0;
			int maxIndex = 0;
			for (int i = 0; i < levels; i++)
			{
				if (intensityCount[i] > maxIntensity)
				{
					maxIntensity = intensityCount[i];
					maxIndex = i;
				}
			}
			dstRow[x] = qRgba(averageR[maxIndex] / maxIntensity, averageG[maxIndex] / maxIntensity, averageB[maxIndex] / maxIntensity, qAlpha(srcRows[y][x]));
		}
	}

	dst = dst.convertToFormat(image.format());
	
	delete [] scratch;
	delete [] srcRows;
	
	return dst;
}

