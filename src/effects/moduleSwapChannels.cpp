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


#include "moduleSwapChannels.h"
#include "globals.h"
#include <QDebug>

EffectModuleSwapChannels::EffectModuleSwapChannels(QObject * parent) : QObject(parent)
{
	
}

EffectModuleSwapChannels::~EffectModuleSwapChannels()
{
	
}

QList<EffectBase::ParameterCluster> EffectModuleSwapChannels::getListOfParameterClusters()
{
	EffectBase::ParameterCluster elem;
	QList<EffectBase::ParameterCluster> cluster = QList<EffectBase::ParameterCluster>();

	elem.displayName = QString(tr("RGB to"));
	elem.controlType = QString("combobox");
	elem.parameterName = QString("RGBto");
	elem.parameterDefaultValue = QVariant(0);
	elem.parameterValue = Globals::prefs->fetchSpecificParameter("EffectModuleSwapChannels", "RGBto", elem.parameterDefaultValue);
	QStringList list = QStringList();
	list.append(QString(tr("RGB")));
	list.append(QString(tr("RBG")));
	list.append(QString(tr("GRB")));
	list.append(QString(tr("GBR")));
	list.append(QString(tr("BRG")));
	list.append(QString(tr("BGR")));
	elem.parameterMinValue = QVariant(list);
	cluster.append(elem);

	return cluster;
}

void EffectModuleSwapChannels::saveEffectParameters(QList<EffectBase::ParameterCluster> parameters)
{
	for (const EffectBase::ParameterCluster & elem : parameters)
	{
		Globals::prefs->storeSpecificParameter("EffectModuleSwapChannels", elem.parameterName, elem.parameterValue);
	}
}

QImage EffectModuleSwapChannels::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	int rgbto = 0;
	for (const EffectBase::ParameterCluster & elem : parameters)
	{
		if (elem.parameterName == "RGBto")
		{
			rgbto = elem.parameterValue.toInt();
		}
		else
		{
			qDebug() << "Invalid parameter" << elem.parameterName;
		}
	}
	
	bool hasAlpha = image.hasAlphaChannel();
	QImage dst;
	if (hasAlpha)
	{
		dst = image.convertToFormat(QImage::Format_ARGB32).copy();
	}
	else
	{
		dst = image.convertToFormat(QImage::Format_RGB32).copy();
	}
	
	for (int y=0; y<dst.height(); y++)
	{
		for (int x=0; x<dst.width(); x++)
		{
			QRgb iP = dst.pixel(x,y);
			
			if (hasAlpha)
			{
				switch(rgbto)
				{
					case 0: dst.setPixel(x, y, qRgba(qRed(iP), qGreen(iP), qBlue(iP), qAlpha(iP))); break;
					case 1: dst.setPixel(x, y, qRgba(qRed(iP), qBlue(iP), qGreen(iP), qAlpha(iP))); break;
					case 2: dst.setPixel(x, y, qRgba(qGreen(iP), qRed(iP), qBlue(iP), qAlpha(iP))); break;
					case 3: dst.setPixel(x, y, qRgba(qGreen(iP), qBlue(iP), qRed(iP), qAlpha(iP))); break;
					case 4: dst.setPixel(x, y, qRgba(qBlue(iP), qRed(iP), qGreen(iP), qAlpha(iP))); break;
					case 5: dst.setPixel(x, y, qRgba(qBlue(iP), qGreen(iP), qRed(iP), qAlpha(iP))); break;
					default: break;
				}
			}
			else
			{
				switch(rgbto)
				{
					case 0: dst.setPixel(x, y, qRgb(qRed(iP), qGreen(iP), qBlue(iP))); break;
					case 1: dst.setPixel(x, y, qRgb(qRed(iP), qBlue(iP), qGreen(iP))); break;
					case 2: dst.setPixel(x, y, qRgb(qGreen(iP), qRed(iP), qBlue(iP))); break;
					case 3: dst.setPixel(x, y, qRgb(qGreen(iP), qBlue(iP), qRed(iP))); break;
					case 4: dst.setPixel(x, y, qRgb(qBlue(iP), qRed(iP), qGreen(iP))); break;
					case 5: dst.setPixel(x, y, qRgb(qBlue(iP), qGreen(iP), qRed(iP))); break;
					default: break;
				}
			}
		}
	}
	
	dst = dst.convertToFormat(image.format());
	
	return dst;
}

