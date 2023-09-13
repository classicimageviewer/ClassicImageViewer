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


#include "moduleReduceNoise.h"
#include "globals.h"
#include <QDebug>

#include "helpersGM.h"

EffectModuleReduceNoise::EffectModuleReduceNoise(QObject * parent) : QObject(parent)
{
	
}

EffectModuleReduceNoise::~EffectModuleReduceNoise()
{
	
}

bool EffectModuleReduceNoise::available()
{
#if defined(HAS_GMAGICK)
	return true;
#else
	return false;
#endif
}

QList<EffectBase::ParameterCluster> EffectModuleReduceNoise::getListOfParameterClusters()
{
	return QList<EffectBase::ParameterCluster>();
}

void EffectModuleReduceNoise::saveEffectParameters(QList<EffectBase::ParameterCluster> parameters)
{
	Q_UNUSED(parameters);
}

QImage EffectModuleReduceNoise::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	Q_UNUSED(parameters);
#if defined(HAS_GMAGICK)
	Magick::Image * mImg = convertQImageToMImage(image);
	mImg->reduceNoise();
	return convertMImageToQImage(mImg);
#else
	return image;
#endif
}

