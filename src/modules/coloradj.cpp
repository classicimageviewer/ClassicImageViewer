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

QImage ColorAdjust::AdjustColor(const QImage src, double valueOfBrightness, double valueOfContrast, double valueOfGamma, double valueOfSaturation, double valueOfHue, double valueOfRed, double valueOfGreen, double valueOfBlue)
{
	QImage dst = src.convertToFormat(QImage::Format_RGB32).copy();

	// HSL: H must be in the range of 0..359; S & L must be in the range of 0..255
	int hLUT[360];
	int sLUT[256];
	int lLUT[256];
	int rgbLUT[3][256];
	int rgbSH[3];
	rgbSH[0] = std::round(valueOfRed * 255.0);
	rgbSH[1] = std::round(valueOfGreen * 255.0);
	rgbSH[2] = std::round(valueOfBlue * 255.0);
	for (int c=0; c<3; c++)
	{
		for (int i=0; i<256; i++)
		{
			int val = i + rgbSH[c];
			rgbLUT[c][i] = qBound(0, val, 255);
		}
	}
	for (int i=0; i<360; i++)
	{
		int shift = std::round(valueOfHue * 180);
		int val = i;
		val += shift;
		if (val < 0) val += 360;
		val %= 360;
		hLUT[i] = val;
	}
	for (int i=0; i<256; i++)
	{
		
		sLUT[i] = std::round(qBound(0.0, i*valueOfSaturation, 255.0));
		double val = i / 255.0;
		val *= valueOfBrightness;
		val -= 0.5;
		val *= valueOfContrast;
		val += 0.5;
		if (val < 0) val = 0;
		val = std::pow(val, 1.0/valueOfGamma);
		val *= 255.0;
		lLUT[i] = std::round(qBound(0.0, val, 255.0));
	}
	
	#pragma omp parallel for schedule(dynamic, 1)
	for (int y=0; y<dst.height(); y++)
	{
		QRgb* row = reinterpret_cast<QRgb *>(dst.scanLine(y));
		for (int x=0; x<dst.width(); x++)
		{
			QColor rgbC = QColor(row[x]);	//dst.pixel(x,y));
			QColor hslC = rgbC.toHsl();
			int h,s,l;
			hslC.getHsl(&h, &s, &l);
			h = hLUT[h];
			s = sLUT[s];
			l = lLUT[l];
			hslC.setHsl(h, s, l);
			QRgb pC = hslC.toRgb().rgb();
			row[x] = qRgb(rgbLUT[0][qRed(pC)], rgbLUT[1][qGreen(pC)], rgbLUT[2][qBlue(pC)]);
			//dst.setPixel(x, y, qRgb(rgbLUT[0][qRed(pC)], rgbLUT[1][qGreen(pC)], rgbLUT[2][qBlue(pC)]));
		}
	}

	dst = dst.convertToFormat(src.format());

	if (src.hasAlphaChannel())
	{
		dst.setAlphaChannel(src.convertToFormat(QImage::Format_Alpha8));
	}
	return dst;
}

