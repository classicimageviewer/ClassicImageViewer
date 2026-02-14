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
// 1) rename file
// 2) rename MODULETEMPLATE_H
// 3) rename class
// 4) change parameter(s)
// 5) implement the effect in the .cpp file
// 6) add to effectHub.cpp (#include and list.append())

#ifndef MODULETEMPLATE_H
#define MODULETEMPLATE_H

#include "effectBase.h"

#include <QObject>
#include <QVariant>
#include <QImage>
#include <QList>
#include <QStringList>

class EffectModuleTemplate : public QObject, public EffectBase
{
	Q_OBJECT
public:
	EffectModuleTemplate(QObject * parent = NULL);
	~EffectModuleTemplate() override;
	
	bool available() override {return true;};
	QString getName() override  {return QString(tr("Template"));};		// displayed effect name
	bool previewModeIsZoom() override  {return true;};			// select between zoom or downscaled preview
	
	QList<EffectBase::ParameterCluster> getListOfParameterClusters() override;
	QImage applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters) override;
};

#endif //MODULETEMPLATE_H
