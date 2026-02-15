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


#include "coloradj.h"
#include "globals.h"
#include <cmath>
#include <QDebug>
#include <omp.h>

QImage ColorAdjust::AdjustColor(const QImage src, double valueOfBrightness, double valueOfContrast, double valueOfGamma, double valueOfExposure, double valueOfSaturation, double valueOfHue, double valueOfRed, double valueOfGreen, double valueOfBlue)
{
	QImage dst = src.convertToFormat(QImage::Format_RGB32).copy();

	// HSL: H must be in the range of 0..359; S & L must be in the range of 0..255
	int hueLUT[360];
	int satLUT[256];
	int bcgLUT[256];	// brightness, contrast, gamma
	int rgbLUT[3][256];
	int rgbShift[3];
	rgbShift[0] = std::round(valueOfRed * 255.0);
	rgbShift[1] = std::round(valueOfGreen * 255.0);
	rgbShift[2] = std::round(valueOfBlue * 255.0);
	
	bool needHSL = ((valueOfHue != 0) || (valueOfSaturation != 0));
	
	valueOfContrast += 1.0;
	valueOfSaturation += 1.0;

	for (int i=0; i<360; i++)
	{
		int shift = std::round(valueOfHue * 180);
		int val = i;
		val += shift;
		if (val < 0) val += 360;
		val %= 360;
		hueLUT[i] = val;
	}
	for (int i=0; i<256; i++)
	{
		for (int c=0; c<3; c++)
		{
			int val = i + rgbShift[c];
			rgbLUT[c][i] = qBound(0, val, 255);
		}
		
		satLUT[i] = std::round(qBound(0.0, i*valueOfSaturation, 255.0));
		
		double val = i / 255.0;
		if (valueOfBrightness <= 0)
		{
			val *= (valueOfBrightness + 1.0);
		}
		else
		{
			val -= 1.0;
			val *= (1.0 - valueOfBrightness);
			val += 1.0;
		}
		val -= 0.5;
		val *= valueOfContrast;
		val += 0.5;
		if (val < 0) val = 0;
		val = std::pow(val, 1.0/valueOfGamma);
		val *= std::exp2(valueOfExposure);
		val *= 255.0;
		bcgLUT[i] = std::round(qBound(0.0, val, 255.0));
	}
	
	omp_set_num_threads(Globals::getThreadCount());
	#pragma omp parallel for schedule(dynamic, 1)
	for (int y=0; y<dst.height(); y++)
	{
		QRgb* row = reinterpret_cast<QRgb *>(dst.scanLine(y));
		for (int x=0; x<dst.width(); x++)
		{
			QRgb rgbQ = row[x];
			rgbQ = qRgb(bcgLUT[qRed(rgbQ)], bcgLUT[qGreen(rgbQ)], bcgLUT[qBlue(rgbQ)]);
			if (needHSL)
			{
				QColor hslC = QColor(rgbQ).toHsl();
				int h,s,l;
				hslC.getHsl(&h, &s, &l);
				h = hueLUT[h];
				s = satLUT[s];
				hslC.setHsl(h, s, l);
				rgbQ = hslC.toRgb().rgb();
			}
			row[x] = qRgb(rgbLUT[0][qRed(rgbQ)], rgbLUT[1][qGreen(rgbQ)], rgbLUT[2][qBlue(rgbQ)]);
		}
	}

	dst = dst.convertToFormat(src.format());

	if (src.hasAlphaChannel())
	{
		dst.setAlphaChannel(src.convertToFormat(QImage::Format_Alpha8));
	}
	return dst;
}

