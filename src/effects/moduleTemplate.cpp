// Copyright (C) 2026 zhuvoy
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


// *************************************
// This is a template for custom effects
// TODO:
// 1) tailor the .h file
// 2) rename file
// 3) apply new class name
// 4) add UI parameter(s) in getListOfParameterClusters()
// 5) retrieve UI parameter(s) in applyEffect()
// 6) implement the effect in applyEffect()

#include "moduleTemplate.h"
#include "globals.h"
#include <QDebug>


EffectModuleTemplate::EffectModuleTemplate(QObject * parent) : QObject(parent)
{
	setModuleName("EffectModuleTemplate");
}

EffectModuleTemplate::~EffectModuleTemplate()
{
	
}

QList<EffectBase::ParameterCluster> EffectModuleTemplate::getListOfParameterClusters()
{
	QList<EffectBase::ParameterCluster> cluster;
	
	/*
	// displayName: shown in the UI; should be translateable eg. 'tr("Parameter name")'
	// paramName: used as an identifier for storing and retrieving the parameter
	cluster += uiParamSpinbox(QString displayName, QString paramName, int defaultValue, int minValue, int maxValue);			// this will add an integer spinbox to the UI
	cluster += uiParamDoubleSpinbox(QString displayName, QString paramName, double defaultValue, double minValue, double maxValue);		// this will add an floating point spinbox to the UI
	cluster += uiParamSlider(QString displayName, QString paramName, int defaultValue, int minValue, int maxValue);				// this will add an integer slider to the UI
	cluster += uiParamSlider10(QString displayName, QString paramName, double defaultValue, double minValue, double maxValue);		// this will add an floating point slider with 0.1 resolution to the UI
	cluster += uiParamSlider100(QString displayName, QString paramName, double defaultValue, double minValue, double maxValue);		// this will add an floating point slider with 0.01 resolution to the UI
	cluster += uiParamSlider1000(QString displayName, QString paramName, double defaultValue, double minValue, double maxValue);		// this will add an floating point slider with 0.001 resolution to the UI
	cluster += uiParamCheckbox(QString displayName, QString paramName, bool defaultValue);							// this will add a checkbox to the UI
	
	QStringList list = QStringList();													// the list of the combobox
	list.append(QString(tr("List item 1")));
	list.append(QString(tr("List item 2")));  
	cluster += uiParamCombobox(QString displayName, QString paramName, int defaultValue, list);						// this will add a combobox (drop-down list) to the UI
	*/
	
	return cluster;
}

#include <QElapsedTimer>

QImage EffectModuleTemplate::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	Q_UNUSED(parameters);		// delete if there are any parameters
	/*
	int integerParameter = getParamIntValue(parameters, QString paramName, int defaultValue);						// retrieve integer value of Spinbox, Slider, Checkbox and Combobox
	double doubleParameter = getParamDoubleValue(parameters, QString paramName, double defaultValue);					// retrieve floating point value of DoubleSpinbox, Slider10, Slider100 and Slider1000
	*/
	
	bool hasAlpha = image.hasAlphaChannel();
	QImage src,dst;

	// convert to xRGB32 format if needed
	if (hasAlpha)
	{
		src = image.convertToFormat(QImage::Format_ARGB32).copy();
		dst = src.copy();
	}
	else
	{
		src = image.convertToFormat(QImage::Format_RGB32).copy();
		dst = src.copy();
	}

	// loop through the xRGB32 format pixels
	for (int y = 0; y < image.height(); y++)
	{
		QRgb* srcRow = reinterpret_cast<QRgb *>(src.scanLine(y)); 
		QRgb* dstRow = reinterpret_cast<QRgb *>(dst.scanLine(y)); 
		for (int x = 0; x < image.width(); x++)
		{
			uint32_t srcPixel = srcRow[x];
			// custom algorithm...
			dstRow[x] = srcPixel;
		}
	}

	// return with the same format of the input, if needed
	dst = dst.convertToFormat(image.format());
	
	return dst;
}

