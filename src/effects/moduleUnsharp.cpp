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
	
}

EffectModuleUnsharp::~EffectModuleUnsharp()
{
	
}

QList<EffectBase::ParameterCluster> EffectModuleUnsharp::getListOfParameterClusters()
{
	EffectBase::ParameterCluster elem;
	QList<EffectBase::ParameterCluster> cluster = QList<EffectBase::ParameterCluster>();

	elem.displayName = QString(tr("Radius"));
	elem.controlType = QString("slider100");
	elem.parameterName = QString("Radius");
	elem.parameterDefaultValue = QVariant(5.0);
	elem.parameterValue = Globals::prefs->fetchSpecificParameter("EffectModuleUnsharp", "Radius", elem.parameterDefaultValue);
	elem.parameterMinValue = QVariant(0.0);
	elem.parameterMaxValue = QVariant(100.0);
	cluster.append(elem);

	elem.displayName = QString(tr("Amount"));
	elem.controlType = QString("slider");
	elem.parameterName = QString("Amount");
	elem.parameterDefaultValue = QVariant(10);
	elem.parameterValue = Globals::prefs->fetchSpecificParameter("EffectModuleUnsharp", "Amount", elem.parameterDefaultValue);
	elem.parameterMinValue = QVariant(0);
	elem.parameterMaxValue = QVariant(100);
	cluster.append(elem);

	return cluster;
}

void EffectModuleUnsharp::saveEffectParameters(QList<EffectBase::ParameterCluster> parameters)
{
	for (const EffectBase::ParameterCluster & elem : parameters)
	{
		Globals::prefs->storeSpecificParameter("EffectModuleUnsharp", elem.parameterName, elem.parameterValue);
	}
}

QImage EffectModuleUnsharp::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	double radius = 0;
	double amount = 0;
	for (const EffectBase::ParameterCluster & elem : parameters)
	{
		if (elem.parameterName == "Radius")
		{
			radius = elem.parameterValue.toDouble();
		} else
		if (elem.parameterName == "Amount")
		{
			amount = elem.parameterValue.toInt() / 100.0;
		} else
		{
			qDebug() << "Invalid parameter" << elem.parameterName;
		}
	}
	
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

