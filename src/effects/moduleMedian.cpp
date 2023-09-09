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


#include "helpersVips.h"

#include "moduleMedian.h"
#include "globals.h"
#include <QDebug>


EffectModuleMedian::EffectModuleMedian(QObject * parent) : QObject(parent)
{
	
}

EffectModuleMedian::~EffectModuleMedian()
{
	
}

bool EffectModuleMedian::available()
{
#if defined(HAS_VIPS)
	return true;
#else
	return false;
#endif
}

QList<EffectBase::ParameterCluster> EffectModuleMedian::getListOfParameterClusters()
{
	EffectBase::ParameterCluster elem;
	QList<EffectBase::ParameterCluster> cluster = QList<EffectBase::ParameterCluster>();

	elem.displayName = QString(tr("Size"));
	elem.controlType = QString("slider");
	elem.parameterName = QString("Size");
	elem.parameterValue = Globals::prefs->fetchSpecificParameter("EffectModuleMedian", "Size", QVariant(3));
	elem.parameterMinValue = QVariant(3);
	elem.parameterMaxValue = QVariant(15);
	cluster.append(elem);

	return cluster;
}

QImage EffectModuleMedian::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	int size = 0;
	for (EffectBase::ParameterCluster elem : parameters)
	{
		Globals::prefs->storeSpecificParameter("EffectModuleMedian", elem.parameterName, elem.parameterValue);
		
		if (elem.parameterName == "Size")
		{
			size = elem.parameterValue.toInt();
		} else
		{
			qDebug() << "Invalid parameter" << elem.parameterName;
		}
	}
#if defined(HAS_VIPS)
	VImage vImg = convertQImageToVImage(image);
	vImg = vImg.median(size);
	return convertVImageToQImage(vImg, image.hasAlphaChannel());
#else
	return image;
#endif
}

