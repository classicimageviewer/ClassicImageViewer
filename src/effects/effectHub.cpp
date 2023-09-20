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


#include "effectHub.h"
#include <QDebug>
#include "effects/moduleSharpen.h"
#include "effects/moduleBlur.h"
#include "effects/moduleUnsharp.h"
#include "effects/moduleSepia.h"
#include "effects/moduleMedian.h"
#include "effects/moduleHistEqual.h"
#include "effects/moduleSobel.h"
#include "effects/moduleCharcoal.h"
#include "effects/moduleDespeckle.h"
#include "effects/moduleNormalize.h"
#include "effects/moduleReduceNoise.h"
#include "effects/moduleSpread.h"
#include "effects/moduleSwapChannels.h"
#include "effects/moduleColorAdjust.h"

EffectHub::EffectHub(QObject * parent) : QObject(parent)
{
	modules = QList<EffectBase*>();
	moduleNames = QStringList();
	
	QList<EffectBase*> list;
	list.append(new EffectModuleSharpen());
	list.append(new EffectModuleBlur());
	list.append(new EffectModuleUnsharp());
	list.append(new EffectModuleSepia());
	list.append(new EffectModuleMedian());
	list.append(new EffectModuleHistEqual());
	list.append(new EffectModuleSobel());
	list.append(new EffectModuleCharcoal());
	list.append(new EffectModuleDespeckle());
	list.append(new EffectModuleNormalize());
	list.append(new EffectModuleReduceNoise());
	list.append(new EffectModuleSpread());
	list.append(new EffectModuleSwapChannels());
	list.append(new EffectModuleColorAdjust());
	
	
	
	for (EffectBase* item : list)
	{
		if (item->available())
		{
			modules.append(item);
			moduleNames.append(item->getName());
		}
		else
		{
			delete item;
		}
	}
}

EffectHub::~EffectHub()
{
	for (EffectBase* item : modules)
	{
		delete item;
	}
}

bool EffectHub::checkId(int effectId)
{
	if (effectId < 0) return false;
	if (effectId >= modules.length()) return false;
	return true;
}

QStringList EffectHub::getEffects()
{
	return moduleNames;
}

bool EffectHub::previewModeIsZoom(int effectId)
{
	if (!checkId(effectId)) return false;
	return modules.at(effectId)->previewModeIsZoom();
}

QList<EffectBase::ParameterCluster> EffectHub::getListOfParameterClusters(int effectId)
{
	if (!checkId(effectId)) return QList<EffectBase::ParameterCluster>();
	return modules.at(effectId)->getListOfParameterClusters();
}

void EffectHub::saveEffectParameters(int effectId, QList<EffectBase::ParameterCluster> parameters)
{
	if (!checkId(effectId)) return;
	modules.at(effectId)->saveEffectParameters(parameters);
}

QImage EffectHub::applyEffect(int effectId, QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	if (!checkId(effectId)) return image;
	return modules.at(effectId)->applyEffect(image, parameters);
}

