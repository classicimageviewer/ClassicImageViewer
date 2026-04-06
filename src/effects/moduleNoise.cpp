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

#include "moduleNoise.h"
#include "globals.h"
#include <QDebug>
#include <cmath>


EffectModuleNoise::EffectModuleNoise(QObject * parent) : QObject(parent)
{
	setModuleName("EffectModuleNoise");
	rngState = 1;
}

EffectModuleNoise::~EffectModuleNoise()
{
	
}

QList<EffectBase::ParameterCluster> EffectModuleNoise::getListOfParameterClusters()
{
	QList<EffectBase::ParameterCluster> cluster;
	
	QStringList list = QStringList();
	list.append(QString(tr("Gaussian")));
	list.append(QString(tr("Gaussian RGB")));
	list.append(QString(tr("Uniform")));
	list.append(QString(tr("Uniform RGB")));
	list.append(QString(tr("Poisson")));
	list.append(QString(tr("Salt-and-pepper")));
	cluster += uiParamCombobox(tr("Noise"), "Noise", 0, list);
	
	cluster += uiParamSlider(tr("Amount"), "Amount", 10, 1, 100);
	
	return cluster;
}

float EffectModuleNoise::rngParkMiller()
{
	rngState = ((uint64_t)rngState * 48271) % 2147483647;
	return rngState / 2147483647.0f;
}

int EffectModuleNoise::gaussian(int amount)
{
	if (gaussianState)
	{
		gaussianState = false;
		return gaussianNext;
	}
	else
	{
		gaussianState = true;
		const float pi2 = 2.0f * M_PI;
		float u1, u2;
		do {
			u1 = rngParkMiller();
		} while (u1 == 0);
		u2 = rngParkMiller();
		
		float mag = amount * std::sqrt(-2.0f * log(u1));
		gaussianNext  = std::round(mag * std::cos(pi2 * u2));
		return std::round(mag * std::sin(pi2 * u2));
	}
}

int EffectModuleNoise::uniform(int amount)
{
	float noise = rngParkMiller();
	noise = noise * 2 - 1;
	return std::round(noise * amount);
}

int EffectModuleNoise::poisson(int amount, float intensity)
{
	float p = 0.0f;
	if (intensity > 0.0f)
	{
		p = -1.0f;
		float expLambda = std::exp(-intensity);
		float product = 1;
		do {
			p += 1.0f;
			product *= rngParkMiller();
		} while(product > expLambda);
	}
	return std::round((p - intensity)*amount);
}

int EffectModuleNoise::saltAndPepper(int amount)
{
	float noise = rngParkMiller() - 0.5f;
	if (std::fabs(noise) > (amount / 1000.0f)) return 0;
	return ((noise > 0.0f) ? 300:-300);
}


QImage EffectModuleNoise::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	int noise = getParamIntValue(parameters, "Noise", 0);
	int amount = getParamIntValue(parameters, "Amount", 10);
	
	bool hasAlpha = image.hasAlphaChannel();
	QImage src,dst;
	
	// convert to xRGB32 format if needed
	if (hasAlpha)
	{
		src = image.convertToFormat(QImage::Format_ARGB32).copy();
		dst = src.copy();
	}
	else
	{
		src = image.convertToFormat(QImage::Format_RGB32).copy();
		dst = src.copy();
	}
	
	gaussianState = false;
	
	// seed
	rngState = 1;
	for (int y = 0; y < image.height(); y += image.height()/4)
	{
		const QRgb* srcRow = reinterpret_cast<const QRgb *>(src.constScanLine(y)); 
		for (int x = 0; x < image.width(); x += image.width()/4)
		{
			uint32_t srcPixel = srcRow[x];
			if (srcPixel == 0) srcPixel = 1;
			srcPixel *= (y + 1);
			srcPixel *= (x + 1);
			rngState += srcPixel;
			rngParkMiller();
		}
	}
	
	// loop through the xRGB32 format pixels
	for (int y = 0; y < image.height(); y++)
	{
		const QRgb* srcRow = reinterpret_cast<const QRgb *>(src.constScanLine(y)); 
		QRgb* dstRow = reinterpret_cast<QRgb *>(dst.scanLine(y)); 
		for (int x = 0; x < image.width(); x++)
		{
			const QRgb srcPixel = srcRow[x];
			int nRGB[3];
			switch(noise)
			{
				default:
				case 0:
					nRGB[2] = nRGB[1] = nRGB[0] = gaussian(amount);
					break;
				case 1:
					nRGB[0] = gaussian(amount);
					nRGB[1] = gaussian(amount);
					nRGB[2] = gaussian(amount);
					break;
				case 2:
					nRGB[2] = nRGB[1] = nRGB[0] = uniform(amount);
					break;
				case 3:
					nRGB[0] = uniform(amount);
					nRGB[1] = uniform(amount);
					nRGB[2] = uniform(amount);
					break;
				case 4:
					nRGB[2] = nRGB[1] = nRGB[0] =  poisson(amount, (qRed(srcPixel) + qGreen(srcPixel) + qBlue(srcPixel)) / 765.0f);
					break;
				case 5:
					nRGB[2] = nRGB[1] = nRGB[0] = saltAndPepper(amount);
					break;
			}
			dstRow[x] = qRgba(qBound(0, (qRed(srcPixel) + nRGB[0]), 255), qBound(0, (qGreen(srcPixel) + nRGB[1]), 255), qBound(0, (qBlue(srcPixel) + nRGB[2]), 255),  qAlpha(srcPixel));
		}
	}
	
	dst = dst.convertToFormat(image.format());
	
	return dst;
}

