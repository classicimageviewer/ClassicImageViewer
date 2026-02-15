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

#include "moduleLocalColorCorrection.h"
#include "globals.h"
#include <QDebug>
#include <QGraphicsBlurEffect>
#include <cmath>


EffectModuleLocalColorCorrection::EffectModuleLocalColorCorrection(QObject * parent) : QObject(parent)
{
	setModuleName("EffectModuleLocalColorCorrection");
}

EffectModuleLocalColorCorrection::~EffectModuleLocalColorCorrection()
{
	
}

QList<EffectBase::ParameterCluster> EffectModuleLocalColorCorrection::getListOfParameterClusters()
{
	QList<EffectBase::ParameterCluster> cluster;
	
	cluster += uiParamSlider(tr("Radius"), "Radius", 10, 1, 100);
	cluster += uiParamSlider(tr("Strength"), "Strength", 100, 1, 100);
	
	return cluster;
}

#include <QElapsedTimer>

QImage EffectModuleLocalColorCorrection::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	int radius = getParamIntValue(parameters, "Radius", 10);
	int strength = getParamIntValue(parameters, "Strength", 100);
	
	strength = strength * 10.24;
	
	bool hasAlpha = image.hasAlphaChannel();
	QImage dst;

	if (hasAlpha)
	{
		dst = image.convertToFormat(QImage::Format_ARGB32).copy();
	}
	else
	{
		dst = image.convertToFormat(QImage::Format_RGB32).copy();
	}
	
	
	
	QImage luminance = image.convertToFormat(QImage::Format_Grayscale8);
	
	QImage luminanceExtended(image.size() + QSize(2*radius,2*radius), QImage::Format_Grayscale8);
	#pragma omp parallel for schedule(static, 1)
	for (int y = 0; y < image.height() + 2*radius; y++)
	{
		int yr = abs(y - radius);
		if (yr >= image.height())
		{
			yr = 2*image.height() - yr - 1;
		}
		uint8_t * sRow = luminance.scanLine(yr);
		uint8_t * dRow = luminanceExtended.scanLine(y);
		for (int x = 0; x < image.width() + 2*radius; x++)
		{
			int xr = abs(x - radius);
			if (xr >= image.width())
			{
				xr = 2*image.width() - xr - 1;
			}
			dRow[x] = sRow[xr];
		}
	}
	
	QGraphicsBlurEffect *e = new QGraphicsBlurEffect();
	e->setBlurRadius(radius);
	e->setBlurHints(QGraphicsBlurEffect::QualityHint);
	QGraphicsScene scene;
	QGraphicsPixmapItem item;
	item.setPixmap(QPixmap::fromImage(luminanceExtended));
	item.setGraphicsEffect(e);
	scene.addItem(&item);
	QImage mask(image.size(), QImage::Format_Grayscale8);
	mask.fill(0);
	QPainter ptr(&mask);
	scene.render(&ptr, QRectF(), QRectF(radius, radius, image.width(), image.height()));
	auto lut = new uint8_t[256][256];
	
	#pragma omp parallel for schedule(static, 16)
	for (int j = 0; j < 256; j++)
	{
		float q = j / 255.0f;
		q = 2.0f*q - 1.0f;
		for (int i = 0; i < 256; i++)
		{
			float p = i / 255.0f;
			lut[j][i] = std::round(qBound(0.0f, std::pow(p, std::pow(2.0f, q)), 1.0f) * 255.0f);
		}
	}

	#pragma omp parallel for schedule(static, 1)
	for (int y = 0; y < image.height(); y++)
	{
		uint8_t * luminanceRow = luminance.scanLine(y);
		uint8_t * maskRow = mask.scanLine(y);
		QRgb* row = reinterpret_cast<QRgb *>(dst.scanLine(y)); 
		for (int x = 0; x < image.width(); x++)
		{
			QRgb pixel = row[x];
			int rgb[3],a;
			a = qAlpha(pixel);
			rgb[0] = qRed(pixel);
			rgb[1] = qGreen(pixel);
			rgb[2] = qBlue(pixel);
			int value = lut[maskRow[x]][luminanceRow[x]];
			for (int c = 0; c < 3; c++)
			{
				rgb[c] = ((qBound(0, ((value * (rgb[c] + luminanceRow[x])) / (luminanceRow[x] + 1) + rgb[c] - luminanceRow[x])/2, 255) * strength) + ((1024 - strength) * rgb[c])) / 1024;
			}
			row[x] = qRgba(rgb[0], rgb[1], rgb[2], a);
		}
	}
	
	delete [] lut;
	
	dst = dst.convertToFormat(image.format());
	
	return dst;
}

