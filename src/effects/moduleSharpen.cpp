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
	setModuleName("EffectModuleSharpen");
}

EffectModuleSharpen::~EffectModuleSharpen()
{
	
}

QList<EffectBase::ParameterCluster> EffectModuleSharpen::getListOfParameterClusters()
{
	QList<EffectBase::ParameterCluster> cluster;
	
	cluster += uiParamSlider(tr("Strength"), "Strength", 5, 1, 100);
	
	return cluster;
}

QImage EffectModuleSharpen::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	double strength = getParamDoubleValue(parameters, "Strength", 0.0) / 100.0; // 0.01 .. 1.0
	
	return Sharpener::Sharpen(image, strength);
}

