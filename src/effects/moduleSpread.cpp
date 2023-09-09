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


#include "moduleSpread.h"
#include "globals.h"
#include <QDebug>

#include "helpersGM.h"

EffectModuleSpread::EffectModuleSpread(QObject * parent) : QObject(parent)
{
	
}

EffectModuleSpread::~EffectModuleSpread()
{
	
}

bool EffectModuleSpread::available()
{
#if defined(HAS_GMAGICK)
	return true;
#else
	return false;
#endif
}

QList<EffectBase::ParameterCluster> EffectModuleSpread::getListOfParameterClusters()
{
	EffectBase::ParameterCluster elem;
	QList<EffectBase::ParameterCluster> cluster = QList<EffectBase::ParameterCluster>();

	elem.displayName = QString(tr("Spread"));
	elem.controlType = QString("slider");
	elem.parameterName = QString("Spread");
	elem.parameterValue = Globals::prefs->fetchSpecificParameter("EffectModuleSpread", "Spread", QVariant(1));
	elem.parameterMinValue = QVariant(1);
	elem.parameterMaxValue = QVariant(100);
	cluster.append(elem);

	return cluster;
}

QImage EffectModuleSpread::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	int spread = 0;
	for (EffectBase::ParameterCluster elem : parameters)
	{
		Globals::prefs->storeSpecificParameter("EffectModuleSpread", elem.parameterName, elem.parameterValue);
		
		if (elem.parameterName == "Spread")
		{
			spread = elem.parameterValue.toInt();
		} else
		{
			qDebug() << "Invalid parameter" << elem.parameterName;
		}
	}
#if defined(HAS_GMAGICK)
	Magick::Image * mImg = convertQImageToMImage(image);
	mImg->spread(spread);
	return convertMImageToQImage(mImg);
#else
	return image;
#endif
}

