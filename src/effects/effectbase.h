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
public:
	EffectBase() {};
	
	virtual ~EffectBase() {};
	
	virtual bool available() = 0;
	virtual QString getName() = 0;
	virtual bool previewModeIsZoom() = 0;
	
	typedef struct {
		QString displayName;
		QString controlType;
		QString parameterName;
		QVariant parameterValue;
		QVariant parameterMinValue;
		QVariant parameterMaxValue;
	} ParameterCluster;
	
	virtual QList<EffectBase::ParameterCluster> getListOfParameterClusters() = 0;
	virtual QImage applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters) = 0;
};

#endif //EFFECTBASE_H
