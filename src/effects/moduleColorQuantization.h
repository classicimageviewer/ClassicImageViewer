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


#ifndef MODULECOLORQUANTIZATION_H
#define MODULECOLORQUANTIZATION_H

#include "effectBase.h"

#include <QObject>
#include <QImage>
#include <QList>

class EffectModuleColorQuantization : public QObject, public EffectBase
{
	Q_OBJECT
private:
	QRgb palette[256];
	int paletteLength;
	int colorError(QRgb a, QRgb b);
	void webSafeGetPalette(QImage image, int colors);
	void sortedGetPalette(QImage image, int colors);
public:
	EffectModuleColorQuantization(QObject * parent = NULL);
	~EffectModuleColorQuantization() override;
	
	bool available() override  {return true;};
	QString getName() override  {return QString(tr("Color Quantization"));};
	bool previewModeIsZoom() override  {return false;};
	
	QList<EffectBase::ParameterCluster> getListOfParameterClusters() override;
	QImage applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters) override;
};

#endif //MODULECOLORQUANTIZATION_H
