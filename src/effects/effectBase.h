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


#ifndef EFFECTBASE_H
#define EFFECTBASE_H

#include <QObject>
#include <QVariant>
#include <QImage>
#include <QList>

class EffectBase
{
private:
	QString moduleName;
public:
	EffectBase() {};
	
	typedef struct {
		QString displayName;
		QString controlType;
		QString parameterName;
		QVariant parameterValue;
		QVariant parameterDefaultValue;
		QVariant parameterMinValue;
		QVariant parameterMaxValue;
	} ParameterCluster;
	
	void setModuleName(QString name);
	QString getModuleName(void);
	void saveEffectParameters(QList<EffectBase::ParameterCluster> parameters);
	
	ParameterCluster uiParamSpinbox(QString displayName, QString paramName, int defaultValue, int minValue, int maxValue);
	ParameterCluster uiParamDoubleSpinbox(QString displayName, QString paramName, double defaultValue, double minValue, double maxValue);
	ParameterCluster uiParamSlider(QString displayName, QString paramName, int defaultValue, int minValue, int maxValue);
	ParameterCluster uiParamSlider10(QString displayName, QString paramName, double defaultValue, double minValue, double maxValue);
	ParameterCluster uiParamSlider100(QString displayName, QString paramName, double defaultValue, double minValue, double maxValue);
	ParameterCluster uiParamSlider1000(QString displayName, QString paramName, double defaultValue, double minValue, double maxValue);
	ParameterCluster uiParamCheckbox(QString displayName, QString paramName, bool defaultValue);
	ParameterCluster uiParamCombobox(QString displayName, QString paramName, int defaultValue, QStringList list);
	
	virtual ~EffectBase() {};
	
	virtual bool available() = 0;
	virtual QString getName() = 0;
	virtual bool previewModeIsZoom() = 0;
	
	virtual QList<EffectBase::ParameterCluster> getListOfParameterClusters() = 0;
	virtual QImage applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters) = 0;
};

#endif //EFFECTBASE_H
