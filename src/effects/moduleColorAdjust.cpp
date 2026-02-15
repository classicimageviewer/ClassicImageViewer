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
#include "lib/coloradj.h"


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
	cluster += uiParamSlider100(tr("Exposure"), "Exposure", 0.0, -10.0, 10.0);
	cluster += uiParamSlider(tr("Saturation"), "Saturation", 0, -100, 100);
	cluster += uiParamSlider(tr("Hue"), "Hue", 0, -180, 180);
	cluster += uiParamSlider(tr("Red"), "Red", 0, -100, 100);
	cluster += uiParamSlider(tr("Green"), "Green", 0, -100, 100);
	cluster += uiParamSlider(tr("Blue"), "Blue", 0, -100, 100);
	
	return cluster;
}

QImage EffectModuleColorAdjust::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	double valueOfBrightness = getParamDoubleValue(parameters, "Brightness", 0.0) / 100.0; // -1.0 .. 1.0
	double valueOfContrast = getParamDoubleValue(parameters, "Contrast", 0.0) / 100.0; // -1.0 .. 1.0
	double valueOfGamma = getParamDoubleValue(parameters, "Gamma", 1.0);
	double valueOfExposure = getParamDoubleValue(parameters, "Exposure", 0.0);
	double valueOfSaturation = getParamDoubleValue(parameters, "Saturation", 0.0) / 100.0; // -1.0 .. 1.0
	double valueOfHue = getParamDoubleValue(parameters, "Hue", 0.0) / 180.0; // -1.0 .. 1.0
	double valueOfRed = getParamDoubleValue(parameters, "Red", 0.0) / 100.0; // -1.0 .. 1.0
	double valueOfGreen = getParamDoubleValue(parameters, "Green", 0.0) / 100.0; // -1.0 .. 1.0
	double valueOfBlue = getParamDoubleValue(parameters, "Blue", 0.0) / 100.0; // -1.0 .. 1.0
	
	return ColorAdjust::AdjustColor(image, valueOfBrightness, valueOfContrast, valueOfGamma, valueOfExposure, valueOfSaturation, valueOfHue, valueOfRed, valueOfGreen, valueOfBlue);
}

