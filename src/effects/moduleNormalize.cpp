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


#include "moduleNormalize.h"
#include "globals.h"
#include <QDebug>

#include "helpersGM.h"

EffectModuleNormalize::EffectModuleNormalize(QObject * parent) : QObject(parent)
{
	
}

EffectModuleNormalize::~EffectModuleNormalize()
{
	
}

bool EffectModuleNormalize::available()
{
#if defined(HAS_GMAGICK)
	return true;
#else
	return false;
#endif
}

QList<EffectBase::ParameterCluster> EffectModuleNormalize::getListOfParameterClusters()
{
	return QList<EffectBase::ParameterCluster>();
}

void EffectModuleNormalize::saveEffectParameters(QList<EffectBase::ParameterCluster> parameters)
{
	Q_UNUSED(parameters);
}

QImage EffectModuleNormalize::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	Q_UNUSED(parameters);
#if defined(HAS_GMAGICK)
	Magick::Image * mImg = convertQImageToMImage(image);
	mImg->normalize();
	return convertMImageToQImage(mImg);
#else
	return image;
#endif
}

