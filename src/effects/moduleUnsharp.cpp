// Copyright (C) 2023 zhuvoy
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


#include "moduleUnsharp.h"
#include "globals.h"
#include <QDebug>
#include <QGraphicsBlurEffect>

EffectModuleUnsharp::EffectModuleUnsharp(QObject * parent) : QObject(parent)
{
	setModuleName("EffectModuleUnsharp");
}

EffectModuleUnsharp::~EffectModuleUnsharp()
{
	
}

QList<EffectBase::ParameterCluster> EffectModuleUnsharp::getListOfParameterClusters()
{
	QList<EffectBase::ParameterCluster> cluster;
	
	cluster += uiParamSlider100(tr("Radius"), "Radius", 5.0, 0.0, 100.0);
	cluster += uiParamSlider(tr("Amount"), "Amount", 10, 0, 100);
	
	return cluster;
}

QImage EffectModuleUnsharp::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	double radius = getParamDoubleValue(parameters, "Radius", 0.0);
	double amount = getParamDoubleValue(parameters, "Amount", 0.0) / 100.0;
	
	QImage dst;
	if (image.hasAlphaChannel())
	{
		dst = image.convertToFormat(QImage::Format_ARGB32).copy();
	}
	else
	{
		dst = image.convertToFormat(QImage::Format_RGB32).copy();
	}
	
	QGraphicsBlurEffect *e = new QGraphicsBlurEffect();
	e->setBlurRadius(radius);
	e->setBlurHints(QGraphicsBlurEffect::QualityHint);
	QGraphicsScene scene;
	QGraphicsPixmapItem item;
	item.setPixmap(QPixmap::fromImage(dst));
	item.setGraphicsEffect(e);
	scene.addItem(&item);
	QImage mask(dst.size(), QImage::Format_ARGB32);
	mask.fill(Qt::transparent);
	QPainter ptr(&mask);
	scene.render(&ptr, QRectF(), QRectF(0, 0, dst.width(), dst.height()));
	bool hasAlpha = dst.hasAlphaChannel();
	
	for (int y=0; y<dst.height(); y++)
	{
		for (int x=0; x<dst.width(); x++)
		{
			QRgb iP = dst.pixel(x,y);
			QRgb mP = mask.pixel(x,y);
			
			int red = qRed(iP) + amount*(qRed(iP) - qRed(mP));
			int green = qGreen(iP) + amount*(qGreen(iP) - qGreen(mP));
			int blue = qBlue(iP) + amount*(qBlue(iP) - qBlue(mP));
			
			if (hasAlpha)
			{
				dst.setPixel(x, y, qRgba(qBound(0, red, 255), qBound(0, green, 255), qBound(0, blue, 255), qAlpha(iP)));
			}
			else
			{
				dst.setPixel(x, y, qRgb(qBound(0, red, 255), qBound(0, green, 255), qBound(0, blue, 255)));
			}
		}
	}
	
	dst = dst.convertToFormat(image.format());
	
	return dst;
}

