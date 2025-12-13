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
	setModuleName("EffectModuleColorAdjust");
}

EffectModuleColorAdjust::~EffectModuleColorAdjust()
{
	
}

QList<EffectBase::ParameterCluster> EffectModuleColorAdjust::getListOfParameterClusters()
{
	QList<EffectBase::ParameterCluster> cluster;
	
	cluster += uiParamSlider(tr("Brightness"), "Brightness", 0, -100, 100);
	cluster += uiParamSlider(tr("Contrast"), "Contrast", 0, -100, 100);
	cluster += uiParamSlider100(tr("Gamma"), "Gamma", 1.0, 0.01, 9.99);
	cluster += uiParamSlider(tr("Saturation"), "Saturation", 0, -100, 100);
	cluster += uiParamSlider(tr("Hue"), "Hue", 0, -180, 180);
	cluster += uiParamSlider(tr("Red"), "Red", 0, -100, 100);
	cluster += uiParamSlider(tr("Green"), "Green", 0, -100, 100);
	cluster += uiParamSlider(tr("Blue"), "Blue", 0, -100, 100);
	
	return cluster;
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

