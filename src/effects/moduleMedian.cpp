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


#include "helpersVips.h"

#include "moduleMedian.h"
#include "globals.h"
#include <QDebug>


EffectModuleMedian::EffectModuleMedian(QObject * parent) : QObject(parent)
{
	setModuleName("EffectModuleMedian");
}

EffectModuleMedian::~EffectModuleMedian()
{
	
}

bool EffectModuleMedian::available()
{
#if defined(HAS_VIPS)
	return true;
#else
	return false;
#endif
}

QList<EffectBase::ParameterCluster> EffectModuleMedian::getListOfParameterClusters()
{
	QList<EffectBase::ParameterCluster> cluster;
	
	cluster += uiParamSlider(tr("Size"), "Size", 3, 3, 15);
	
	return cluster;
}

QImage EffectModuleMedian::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
#if defined(HAS_VIPS)
	int size = getParamIntValue(parameters, "Size", 0);
	
	VImage vImg = convertQImageToVImage(image);
	vImg = vImg.median(size);
	return convertVImageToQImage(vImg, image.hasAlphaChannel());
#else
	Q_UNUSED(parameters);
	return image;
#endif
}

