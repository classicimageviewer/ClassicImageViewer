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
	setModuleName("EffectModuleBlur");
}

EffectModuleBlur::~EffectModuleBlur()
{
	
}

QList<EffectBase::ParameterCluster> EffectModuleBlur::getListOfParameterClusters()
{
	QList<EffectBase::ParameterCluster> cluster;

	cluster += uiParamSlider100(tr("Radius"), "Radius", 5.0, 0.0, 100.0);

	return cluster;
}

QImage EffectModuleBlur::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	double radius = getParamDoubleValue(parameters, "Radius", 0.0);
	
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

