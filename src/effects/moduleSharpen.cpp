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


#include "moduleSharpen.h"
#include "globals.h"
#include <QDebug>
#include "modules/sharpener.h"


EffectModuleSharpen::EffectModuleSharpen(QObject * parent) : QObject(parent)
{
	
}

EffectModuleSharpen::~EffectModuleSharpen()
{
	
}

QList<EffectBase::ParameterCluster> EffectModuleSharpen::getListOfParameterClusters()
{
	EffectBase::ParameterCluster elem;
	QList<EffectBase::ParameterCluster> cluster = QList<EffectBase::ParameterCluster>();

	elem.displayName = QString(tr("Strength"));
	elem.controlType = QString("slider");
	elem.parameterName = QString("Strength");
	elem.parameterDefaultValue = QVariant(5);
	elem.parameterValue = Globals::prefs->fetchSpecificParameter("EffectModuleSharpen", "Strength", elem.parameterDefaultValue);
	elem.parameterMinValue = QVariant(1);
	elem.parameterMaxValue = QVariant(100);
	cluster.append(elem);

	return cluster;
}

void EffectModuleSharpen::saveEffectParameters(QList<EffectBase::ParameterCluster> parameters)
{
	for (const EffectBase::ParameterCluster & elem : parameters)
	{
		Globals::prefs->storeSpecificParameter("EffectModuleSharpen", elem.parameterName, elem.parameterValue);
	}
}

QImage EffectModuleSharpen::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	double strength = 0;
	for (const EffectBase::ParameterCluster & elem : parameters)
	{
		if (elem.parameterName == "Strength")
		{
			strength = elem.parameterValue.toInt() / 100.0; // 0.01 .. 1.0
		} else
		{
			qDebug() << "Invalid parameter" << elem.parameterName;
		}
	}
	
	return Sharpener::Sharpen(image, strength);
}

