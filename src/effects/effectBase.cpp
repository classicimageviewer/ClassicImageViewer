// Copyright (C) 2025 zhuvoy
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


#include "effectBase.h"
#include "globals.h"

void EffectBase::setModuleName(QString name)
{
	moduleName = name;
}

QString EffectBase::getModuleName(void)
{
	return moduleName;
}

void EffectBase::saveEffectParameters(QList<EffectBase::ParameterCluster> parameters)
{
	for (const EffectBase::ParameterCluster & elem : parameters)
	{
		Globals::prefs->storeSpecificParameter(moduleName, elem.parameterName, elem.parameterValue);
	}
}

EffectBase::ParameterCluster EffectBase::uiParamSpinbox(QString displayName, QString paramName, int defaultValue, int minValue, int maxValue)
{
	ParameterCluster elem;
	
	elem.displayName = displayName;
	elem.controlType = QString("spinbox");
	elem.parameterName = paramName;
	elem.parameterDefaultValue = QVariant(defaultValue);
	elem.parameterValue = Globals::prefs->fetchSpecificParameter(moduleName, paramName, elem.parameterDefaultValue);
	elem.parameterMinValue = QVariant(minValue);
	elem.parameterMaxValue = QVariant(maxValue);
	
	return elem;
}

EffectBase::ParameterCluster EffectBase::uiParamDoubleSpinbox(QString displayName, QString paramName, double defaultValue, double minValue, double maxValue)
{
	ParameterCluster elem;
	
	elem.displayName = displayName;
	elem.controlType = QString("doublespinbox");
	elem.parameterName = paramName;
	elem.parameterDefaultValue = QVariant(defaultValue);
	elem.parameterValue = Globals::prefs->fetchSpecificParameter(moduleName, paramName, elem.parameterDefaultValue);
	elem.parameterMinValue = QVariant(minValue);
	elem.parameterMaxValue = QVariant(maxValue);
	
	return elem;
}

EffectBase::ParameterCluster EffectBase::uiParamSlider(QString displayName, QString paramName, int defaultValue, int minValue, int maxValue)
{
	ParameterCluster elem;
	
	elem.displayName = displayName;
	elem.controlType = QString("slider");
	elem.parameterName = paramName;
	elem.parameterDefaultValue = QVariant(defaultValue);
	elem.parameterValue = Globals::prefs->fetchSpecificParameter(moduleName, paramName, elem.parameterDefaultValue);
	elem.parameterMinValue = QVariant(minValue);
	elem.parameterMaxValue = QVariant(maxValue);
	
	return elem;
}

EffectBase::ParameterCluster EffectBase::uiParamSlider10(QString displayName, QString paramName, double defaultValue, double minValue, double maxValue)
{
	ParameterCluster elem;
	
	elem.displayName = displayName;
	elem.controlType = QString("slider10");
	elem.parameterName = paramName;
	elem.parameterDefaultValue = QVariant(defaultValue);
	elem.parameterValue = Globals::prefs->fetchSpecificParameter(moduleName, paramName, elem.parameterDefaultValue);
	elem.parameterMinValue = QVariant(minValue);
	elem.parameterMaxValue = QVariant(maxValue);
	
	return elem;
}

EffectBase::ParameterCluster EffectBase::uiParamSlider100(QString displayName, QString paramName, double defaultValue, double minValue, double maxValue)
{
	ParameterCluster elem;
	
	elem.displayName = displayName;
	elem.controlType = QString("slider100");
	elem.parameterName = paramName;
	elem.parameterDefaultValue = QVariant(defaultValue);
	elem.parameterValue = Globals::prefs->fetchSpecificParameter(moduleName, paramName, elem.parameterDefaultValue);
	elem.parameterMinValue = QVariant(minValue);
	elem.parameterMaxValue = QVariant(maxValue);
	
	return elem;
}

EffectBase::ParameterCluster EffectBase::uiParamSlider1000(QString displayName, QString paramName, double defaultValue, double minValue, double maxValue)
{
	ParameterCluster elem;
	
	elem.displayName = displayName;
	elem.controlType = QString("slider1000");
	elem.parameterName = paramName;
	elem.parameterDefaultValue = QVariant(defaultValue);
	elem.parameterValue = Globals::prefs->fetchSpecificParameter(moduleName, paramName, elem.parameterDefaultValue);
	elem.parameterMinValue = QVariant(minValue);
	elem.parameterMaxValue = QVariant(maxValue);
	
	return elem;
}

EffectBase::ParameterCluster EffectBase::uiParamCheckbox(QString displayName, QString paramName, bool defaultValue)
{
	ParameterCluster elem;
	
	elem.displayName = displayName;
	elem.controlType = QString("checkbox");
	elem.parameterName = paramName;
	elem.parameterDefaultValue = QVariant(defaultValue);
	elem.parameterValue = Globals::prefs->fetchSpecificParameter(moduleName, paramName, elem.parameterDefaultValue);
	
	return elem;
}

EffectBase::ParameterCluster EffectBase::uiParamCombobox(QString displayName, QString paramName, int defaultValue, QStringList list)
{
	ParameterCluster elem;
	
	elem.displayName = displayName;
	elem.controlType = QString("combobox");
	elem.parameterName = paramName;
	elem.parameterDefaultValue = QVariant(defaultValue);
	elem.parameterValue = Globals::prefs->fetchSpecificParameter(moduleName, paramName, elem.parameterDefaultValue);
	elem.parameterMinValue = QVariant(list);	// list passed through here
	
	return elem;
}

int EffectBase::getParamIntValue(QList<EffectBase::ParameterCluster> parameters, QString paramName, int defaultValue)
{
	for (const EffectBase::ParameterCluster & elem : parameters)
	{
		if (elem.parameterName == paramName)
		{
			return elem.parameterValue.toInt();
		}
	}
	return defaultValue;
}

double EffectBase::getParamDoubleValue(QList<EffectBase::ParameterCluster> parameters, QString paramName, double defaultValue)
{
	for (const EffectBase::ParameterCluster & elem : parameters)
	{
		if (elem.parameterName == paramName)
		{
			return elem.parameterValue.toDouble();
		}
	}
	return defaultValue;
}


