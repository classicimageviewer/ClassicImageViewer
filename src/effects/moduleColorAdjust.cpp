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


#include "moduleColorAdjust.h"
#include "globals.h"
#include <QDebug>
#include "modules/coloradj.h"


EffectModuleColorAdjust::EffectModuleColorAdjust(QObject * parent) : QObject(parent)
{
	
}

EffectModuleColorAdjust::~EffectModuleColorAdjust()
{
	
}

QList<EffectBase::ParameterCluster> EffectModuleColorAdjust::getListOfParameterClusters()
{
	EffectBase::ParameterCluster elem;
	QList<EffectBase::ParameterCluster> cluster = QList<EffectBase::ParameterCluster>();
	
	elem.displayName = QString(tr("Brightness"));
	elem.controlType = QString("slider");
	elem.parameterName = QString("Brightness");
	elem.parameterDefaultValue = QVariant(0);
	elem.parameterValue = Globals::prefs->fetchSpecificParameter("EffectModuleColorAdjust", "Brightness", elem.parameterDefaultValue);
	elem.parameterMinValue = QVariant(-100);
	elem.parameterMaxValue = QVariant(100);
	cluster.append(elem);
	
	elem.displayName = QString(tr("Contrast"));
	elem.controlType = QString("slider");
	elem.parameterName = QString("Contrast");
	elem.parameterDefaultValue = QVariant(0);
	elem.parameterValue = Globals::prefs->fetchSpecificParameter("EffectModuleColorAdjust", "Contrast", elem.parameterDefaultValue);
	elem.parameterMinValue = QVariant(-100);
	elem.parameterMaxValue = QVariant(100);
	cluster.append(elem);
	
	elem.displayName = QString(tr("Gamma"));
	elem.controlType = QString("slider100");
	elem.parameterName = QString("Gamma");
	elem.parameterDefaultValue = QVariant(1.0);
	elem.parameterValue = Globals::prefs->fetchSpecificParameter("EffectModuleColorAdjust", "Gamma", elem.parameterDefaultValue);
	elem.parameterMinValue = QVariant(0.01);
	elem.parameterMaxValue = QVariant(9.99);
	cluster.append(elem);
	
	elem.displayName = QString(tr("Saturation"));
	elem.controlType = QString("slider");
	elem.parameterName = QString("Saturation");
	elem.parameterDefaultValue = QVariant(0);
	elem.parameterValue = Globals::prefs->fetchSpecificParameter("EffectModuleColorAdjust", "Saturation", elem.parameterDefaultValue);
	elem.parameterMinValue = QVariant(-100);
	elem.parameterMaxValue = QVariant(100);
	cluster.append(elem);
	
	elem.displayName = QString(tr("Hue"));
	elem.controlType = QString("slider");
	elem.parameterName = QString("Hue");
	elem.parameterDefaultValue = QVariant(0);
	elem.parameterValue = Globals::prefs->fetchSpecificParameter("EffectModuleColorAdjust", "Hue", elem.parameterDefaultValue);
	elem.parameterMinValue = QVariant(-180);
	elem.parameterMaxValue = QVariant(180);
	cluster.append(elem);
	
	elem.displayName = QString(tr("Red"));
	elem.controlType = QString("slider");
	elem.parameterName = QString("Red");
	elem.parameterDefaultValue = QVariant(0);
	elem.parameterValue = Globals::prefs->fetchSpecificParameter("EffectModuleColorAdjust", "Red", elem.parameterDefaultValue);
	elem.parameterMinValue = QVariant(-100);
	elem.parameterMaxValue = QVariant(100);
	cluster.append(elem);
	
	elem.displayName = QString(tr("Green"));
	elem.controlType = QString("slider");
	elem.parameterName = QString("Green");
	elem.parameterDefaultValue = QVariant(0);
	elem.parameterValue = Globals::prefs->fetchSpecificParameter("EffectModuleColorAdjust", "Green", elem.parameterDefaultValue);
	elem.parameterMinValue = QVariant(-100);
	elem.parameterMaxValue = QVariant(100);
	cluster.append(elem);
	
	elem.displayName = QString(tr("Blue"));
	elem.controlType = QString("slider");
	elem.parameterName = QString("Blue");
	elem.parameterDefaultValue = QVariant(0);
	elem.parameterValue = Globals::prefs->fetchSpecificParameter("EffectModuleColorAdjust", "Blue", elem.parameterDefaultValue);
	elem.parameterMinValue = QVariant(-100);
	elem.parameterMaxValue = QVariant(100);
	cluster.append(elem);
	
	return cluster;
}

void EffectModuleColorAdjust::saveEffectParameters(QList<EffectBase::ParameterCluster> parameters)
{
	for (const EffectBase::ParameterCluster & elem : parameters)
	{
		Globals::prefs->storeSpecificParameter("EffectModuleColorAdjust", elem.parameterName, elem.parameterValue);
	}
}

QImage EffectModuleColorAdjust::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	double valueOfBrightness, valueOfContrast, valueOfGamma, valueOfSaturation, valueOfHue, valueOfRed, valueOfGreen, valueOfBlue;
	valueOfGamma = 1.0;
	valueOfBrightness = valueOfContrast = valueOfSaturation = valueOfHue = valueOfRed = valueOfGreen = valueOfBlue = 0.0;
	
	for (const EffectBase::ParameterCluster & elem : parameters)
	{
		if (elem.parameterName == "Brightness")
		{
			valueOfBrightness = elem.parameterValue.toInt() / 100.0; // -1.0 .. 1.0
		} else
		if (elem.parameterName == "Contrast")
		{
			valueOfContrast = elem.parameterValue.toInt() / 100.0; // -1.0 .. 1.0
		} else
		if (elem.parameterName == "Gamma")
		{
			valueOfGamma = elem.parameterValue.toDouble();
		} else
		if (elem.parameterName == "Saturation")
		{
			valueOfSaturation = elem.parameterValue.toInt() / 100.0; // -1.0 .. 1.0
		} else
		if (elem.parameterName == "Hue")
		{
			valueOfHue = elem.parameterValue.toInt() / 180.0; // -1.0 .. 1.0
		} else
		if (elem.parameterName == "Red")
		{
			valueOfRed = elem.parameterValue.toInt() / 100.0; // -1.0 .. 1.0
		} else
		if (elem.parameterName == "Green")
		{
			valueOfGreen = elem.parameterValue.toInt() / 100.0; // -1.0 .. 1.0
		} else
		if (elem.parameterName == "Blue")
		{
			valueOfBlue = elem.parameterValue.toInt() / 100.0; // -1.0 .. 1.0
		} else
		{
			qDebug() << "Invalid parameter" << elem.parameterName;
		}
	}
	
	return ColorAdjust::AdjustColor(image, valueOfBrightness, valueOfContrast, valueOfGamma, valueOfSaturation, valueOfHue, valueOfRed, valueOfGreen, valueOfBlue);
}

