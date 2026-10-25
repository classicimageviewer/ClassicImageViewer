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

#ifndef MODULELOOKUPTABLE_H
#define MODULELOOKUPTABLE_H

#include "effectBase.h"
#include <vector>

class EffectModuleLookupTable : public QObject, public EffectBase
{
	Q_OBJECT
	struct rgbTriplet
	{
		float r, g, b;
		rgbTriplet operator+(const rgbTriplet &o) const
		{
			return { r + o.r, g + o.g, b + o.b };
		}

		rgbTriplet operator*(float s) const
		{
			return { r * s, g * s, b * s };
		}

		friend rgbTriplet operator*(float s, const rgbTriplet &v)
		{
			return v * s;
		}
	};
	
	int cubeSize = 0;
	std::vector<rgbTriplet> cube;
	
	bool loadCubeFile(const QString & filepath);
	bool loadLutFromTexture(const QString & filepath);
	rgbTriplet indexCube(int x, int y, int z);
	QRgb cubeInterpolation(QRgb rgb, bool hasAlpha);
public:
	EffectModuleLookupTable(QObject * parent = NULL);
	~EffectModuleLookupTable() override;

	bool available() override { return true; };
	QString getName() override { return QString(tr("Lookup Table")); };
	bool previewModeIsZoom() override { return false; };

	QList<EffectBase::ParameterCluster> getListOfParameterClusters() override;
	QImage applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters) override;
};

#endif // MODULELOOKUPTABLE_H

