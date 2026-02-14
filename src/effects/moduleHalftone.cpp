// Copyright (C) 2006 Jerry Huxtable
// Copyright (C) 2012 Janne Liljeblad
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


#include "moduleHalftone.h"
#include "globals.h"
#include <QDebug>
#include <cmath>


EffectModuleHalftone::EffectModuleHalftone(QObject * parent) : QObject(parent)
{
	setModuleName("EffectModuleHalftone");
}

EffectModuleHalftone::~EffectModuleHalftone()
{
	
}

bool EffectModuleHalftone::available()
{
	return true;
}

QList<EffectBase::ParameterCluster> EffectModuleHalftone::getListOfParameterClusters()
{
	QList<EffectBase::ParameterCluster> cluster;
	
	cluster += uiParamSlider(tr("Radius"), "Radius", 2, 1, 10);
	cluster += uiParamSlider(tr("Cyan screen angle"), "CyanScreenAngle", 108, -180, 180);
	cluster += uiParamSlider(tr("Magenta screen angle"), "MagentaScreenAngle", 162, -180, 180);
	cluster += uiParamSlider(tr("Yellow screen angle"), "YellowScreenAngle", 90, -180, 180);
	
	return cluster;
}

#include <QElapsedTimer>

QImage EffectModuleHalftone::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	float radius = getParamIntValue(parameters, "Radius", 2);
	float cyanScreenAngle = getParamIntValue(parameters, "CyanScreenAngle", 108)*(M_PI/180.0);
	float magentaScreenAngle = getParamIntValue(parameters, "MagentaScreenAngle", 162)*(M_PI/180.0);
	float yellowScreenAngle = getParamIntValue(parameters, "YellowScreenAngle", 90)*(M_PI/180.0);
	bool hasAlpha = image.hasAlphaChannel();
	QImage src,dst;
	int width  = image.width();
	int height = image.height();

	if (hasAlpha)
	{
		src = image.convertToFormat(QImage::Format_ARGB32).copy();
		dst = QImage(width, height, QImage::Format_ARGB32);
	}
	else
	{
		src = image.convertToFormat(QImage::Format_RGB32).copy();
		dst = QImage(width, height, QImage::Format_RGB32);
	}

	const float gridSize = 2 * radius * 1.414f;
	const float mx[] = {0, -1, 1,  0, 0};
	const float my[] = {0,  0, 0, -1, 1};
	const float halfGridSize = gridSize / 2;
	const float pixelScale = halfGridSize * (1.414f / 65025.f);

	const float sinValues[] = {std::sin(cyanScreenAngle), std::sin(magentaScreenAngle), std::sin(yellowScreenAngle)};
	const float cosValues[] = {std::cos(cyanScreenAngle), std::cos(magentaScreenAngle), std::cos(yellowScreenAngle)};

	#pragma omp parallel for schedule(dynamic, 1)
	for (int y = 0; y < height; y++)
	{
		QRgb* row = reinterpret_cast<QRgb *>(dst.scanLine(y)); 
		for (int x = 0; x < width; x++)
		{
			uint32_t v = 0;
			
			for (int channel = 0; channel < 3; channel++ )
			{
				int shift = 16 - 8*channel;
				float sinValue = sinValues[channel];
				float cosValue = cosValues[channel];

				// Transform x,y into halftone screen coordinate space
				float tX =  x*cosValue + y*sinValue;
				float tY = -x*sinValue + y*cosValue;
				
				// Find the nearest grid point
				float gX = tX - std::fmod(tX-halfGridSize, gridSize) + ((tX < halfGridSize) ? -halfGridSize : halfGridSize);
				float gY = tY - std::fmod(tY-halfGridSize, gridSize) + ((tY < halfGridSize) ? -halfGridSize : halfGridSize);

				float f = 1.0f;

				for (int i = 0; i < 5; i++) 
				{
					// Find neigbouring grid point
					float nX = gX + mx[i]*gridSize;
					float nY = gY + my[i]*gridSize;
					// Transform back into image space
					float sX = nX*cosValue - nY*sinValue;
					float sY = nX*sinValue + nY*cosValue;
					float dx = x - sX;
					float dy = y - sY;
					float R2 = (dx * dx) + (dy * dy);
					// Clamp to the image
					int iX = qBound( 0, static_cast<int>(sX), width - 1);
					int iY = qBound( 0, static_cast<int>(sY), height - 1);
					uint32_t pixel = reinterpret_cast<QRgb *>(src.scanLine(iY))[iX];
					pixel = (pixel >> shift) & 0xFF;
					float l = (65025 - pixel*pixel) * pixelScale;
					if (l*l < R2) continue;
					float R = std::sqrt(R2);
					if (l >= (R + 1.0f))
					{
						f = 0.0f;
						break;
					}
					else
					{
						l = (l - R);
						float f2 =  1.0f - l*l * (3.0f - 2.0f*l);
						if (f2 < f) f = f2;
					}
				}
				v |= static_cast<uint32_t>(255.0f * f) << shift;
			}
			row[x] = v | 0xFF000000UL;
		}
	}

	dst = dst.convertToFormat(image.format());
	
	return dst;
}

