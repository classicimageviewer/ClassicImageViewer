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


#include "moduleColorTemperature.h"
#include "globals.h"
#include <QDebug>
#include <cmath>


EffectModuleColorTemperature::EffectModuleColorTemperature(QObject * parent) : QObject(parent)
{
	setModuleName("EffectModuleColorTemperature");
}

EffectModuleColorTemperature::~EffectModuleColorTemperature()
{
	
}

QList<EffectBase::ParameterCluster> EffectModuleColorTemperature::getListOfParameterClusters()
{
	QList<EffectBase::ParameterCluster> cluster;
	
	cluster += uiParamSlider(tr("Temperature"), "Temperature", 6500, 1000, 12000);
	
	return cluster;
}

QImage EffectModuleColorTemperature::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	int temperature = getParamIntValue(parameters, "Temperature", 6500);
	
	QImage dst = image.convertToFormat(QImage::Format_RGB32).copy();
	
	
	double xd,yd,t;
	double xyz[3];
	double s[3];
	double m[3][3];
	float conversionMatrix[3][3];
	
	t = temperature / 1000.0;
	if (temperature < 6500)
	{
		t = 0.7*t + 1.95;
	}
	if (temperature < 7000)
	{
		xd = -4.6070/(t*t*t) + 2.9678/(t*t) + 0.09911/t + 0.244063;
	}
	else
	{
		xd = -2.0064/(t*t*t) + 1.9018/(t*t) + 0.24748/t + 0.23704;
	}
	yd = -3.0*xd*xd + 2.87*xd - 0.275;
	
	xyz[0] = xd/yd;
	xyz[1] = 1.0;
	xyz[2] = (1.0 - xd - yd) / yd;
	
	s[0] =  6.8916e-1*xyz[0] + -3.2691e-1*xyz[1] + -1.0602e-1*xyz[2];
	s[1] = -6.9317e-1*xyz[0] +  1.3416e+0*xyz[1] +  2.9719e-2*xyz[2];
	s[2] =  4.0161e-3*xyz[0] + -1.4726e-2*xyz[1] +  7.6305e-2*xyz[2];
	
	m[0][0] = s[0]*1.9394e+0;  m[1][0] = s[1]*5.0000e-1; m[2][0] = s[2]*2.5000e+0;
	m[0][1] = s[0]*1.0000e+0;  m[1][1] = s[1]*1.0000e+0; m[2][1] = s[2]*1.0000e+0;
	m[0][2] = s[0]*9.0909e-2;  m[1][2] = s[1]*1.6667e-1; m[2][2] = s[2]*1.3167e+1;
	
	for (int i=0; i<3; i++)
	{
		conversionMatrix[0][i] =  3.2404542*m[i][0] + -1.5371385*m[i][1] + -0.4985314*m[i][2];
		conversionMatrix[1][i] = -0.9692660*m[i][0] +  1.8760108*m[i][1] +  0.0415560*m[i][2];
		conversionMatrix[2][i] =  0.0556434*m[i][0] + -0.2040259*m[i][1] +  1.0572252*m[i][2];
	}
	
	#pragma omp parallel for schedule(dynamic, 1)
	for (int y=0; y<dst.height(); y++)
	{
		QRgb* row = reinterpret_cast<QRgb *>(dst.scanLine(y));
		for (int x=0; x<dst.width(); x++)
		{
			QRgb rgbQ = row[x];
			float rgb[3];
			rgb[0] = qRed(rgbQ) / 255.0f;
			rgb[1] = qGreen(rgbQ) / 255.0f;
			rgb[2] = qBlue(rgbQ) / 255.0f;
			
			for (int i=0; i<3; i++)
			{
				rgb[i] = std::pow(rgb[i],1.0f/2.2f);
			}
			
			for (int i=0; i<3; i++)
			{
				float tmp = conversionMatrix[i][0]*rgb[0] + conversionMatrix[i][1]*rgb[1] + conversionMatrix[i][2]*rgb[2];
				tmp = std::pow(qBound(0.0f, tmp, 1.0f), 2.2f);
				rgb[i] = qBound(0.0f, std::round(tmp * 255.0f), 255.0f);
			}
			
			row[x] = qRgb(static_cast<int>(rgb[0]), static_cast<int>(rgb[1]), static_cast<int>(rgb[2]));
		}
	}
	
	dst = dst.convertToFormat(image.format());
	
	if (image.hasAlphaChannel())
	{
		dst.setAlphaChannel(image.convertToFormat(QImage::Format_Alpha8));
	}
	return dst;
}

