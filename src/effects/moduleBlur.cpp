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


#include "moduleBlur.h"
#include "globals.h"
#include <QDebug>
#include <QGraphicsBlurEffect>

EffectModuleBlur::EffectModuleBlur(QObject * parent) : QObject(parent)
{
	
}

EffectModuleBlur::~EffectModuleBlur()
{
	
}

QList<EffectBase::ParameterCluster> EffectModuleBlur::getListOfParameterClusters()
{
	EffectBase::ParameterCluster elem;
	QList<EffectBase::ParameterCluster> cluster = QList<EffectBase::ParameterCluster>();

	elem.displayName = QString(tr("Radius"));
	elem.controlType = QString("slider100");
	elem.parameterName = QString("Radius");
	elem.parameterValue = Globals::prefs->fetchSpecificParameter("EffectModuleBlur", "Radius", QVariant(5.0));
	elem.parameterMinValue = QVariant(0.0);
	elem.parameterMaxValue = QVariant(100.0);
	cluster.append(elem);

	return cluster;
}

QImage EffectModuleBlur::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	double radius = 0;
	for (EffectBase::ParameterCluster elem : parameters)
	{
		Globals::prefs->storeSpecificParameter("EffectModuleBlur", elem.parameterName, elem.parameterValue);
		
		if (elem.parameterName == "Radius")
		{
			radius = elem.parameterValue.toDouble();
		} else
		{
			qDebug() << "Invalid parameter" << elem.parameterName;
		}
	}
	
	QGraphicsBlurEffect *e = new QGraphicsBlurEffect();
	e->setBlurRadius(radius);
	e->setBlurHints(QGraphicsBlurEffect::QualityHint);
	QGraphicsScene scene;
	QGraphicsPixmapItem item;
	item.setPixmap(QPixmap::fromImage(image));
	item.setGraphicsEffect(e);
	scene.addItem(&item);
	QImage dst(image.size(), QImage::Format_ARGB32);
	dst.fill(Qt::transparent);
	QPainter ptr(&dst);
	scene.render(&ptr, QRectF(), QRectF(0, 0, image.width(), image.height()));
	
	return dst.convertToFormat(image.format());
}

