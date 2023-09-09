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


#include "autocolor.h"
#include "globals.h"
#include <cmath>
#include <QDebug>

QImage AutoColor::Adjust(const QImage src)
{
	/*QImage dst = src.convertToFormat(QImage::Format_RGB32).copy();
	
	unsigned long long int hist[256];
	for (int i=0; i<256; i++)
	{
		hist[i] = 0;
	}
	for (int y=0; y<src.height(); y++)
	{
		for (int x=0; x<src.width(); x++)
		{
			hist[qGray(dst.pixel(x,y))]++;
		}
	}
	
	double mean = 0.0;
	double weight = 0.0;
	for (int i=1; i<256; i++)
	{
		mean += hist[i] * std::log(i/255.0);
		weight += hist[i];
	}
	mean /= weight;
	double gamma = -1.0/mean;

	int LUT[256];

	for (int i=0; i<256; i++)
	{
		if (!std::isfinite(gamma))
		{
			LUT[i] = i;
		}
		else
		{
			int val = std::round(255.0*std::pow(i/255.0, std::sqrt(gamma)));
			LUT[i] = qBound(0, val, 255);
		}
	}

	
	for (int y=0; y<src.height(); y++)
	{
		for (int x=0; x<src.width(); x++)
		{
			QRgb pC = dst.pixel(x,y);
			dst.setPixel(x, y, qRgb(LUT[qRed(pC)], LUT[qGreen(pC)], LUT[qBlue(pC)]));
		}
	}*/
	
	/*QImage dst = src.convertToFormat(QImage::Format_RGB32).copy();
	
	unsigned long long int hist[256];
	for (int i=0; i<256; i++)
	{
		hist[i] = 0;
	}
	for (int y=0; y<src.height(); y++)
	{
		for (int x=0; x<src.width(); x++)
		{
			hist[qGray(dst.pixel(x,y))]++;
		}
	}
	#define HIST_LO_THRES	(0.0025)
	#define HIST_HI_THRES	(0.995)
	int LUT[256];
	int min, max, center;
	min = 0;
	max = 255;
	center = 128;
	for (int i=1; i<256; i++)
	{
		hist[i] += hist[i-1];	// cumulative distribution
	}
	for (int i=0; i<256; i++)
	{
		if (hist[i] <= (hist[255]*HIST_LO_THRES)) min = i;
		if (hist[i] <= (hist[255]*0.73)) center = i;
	}
	for (int i=255; i>=0; i--)
	{
		if (hist[i] >= (hist[255]*HIST_HI_THRES)) max = i;
	}
	
	for (int i=0; i<256; i++)
	{
		if (min == max)
		{
			LUT[i] = i;
		}
		else
		{
			double newCenter = (center-min)*(1.0/(max - min));
			double gamma = 1.0;//std::pow(std::log(0.73)/std::log(newCenter), 1.0/8.0);
			int val = std::round(std::pow((i-min)*(1.0/(max - min)), gamma) * 255.0);
			LUT[i] = qBound(0, val, 255);
		}
	}
	#undef HIST_LO_THRES
	#undef HIST_HI_THRES
	
	for (int y=0; y<src.height(); y++)
	{
		for (int x=0; x<src.width(); x++)
		{
			QRgb pC = dst.pixel(x,y);
			dst.setPixel(x, y, qRgb(LUT[qRed(pC)], LUT[qGreen(pC)], LUT[qBlue(pC)]));
		}
	}*/


	QImage dst = src.convertToFormat(QImage::Format_RGB32).copy();
	unsigned long long int rgbHist[3][256];
	for (int i=0; i<256; i++)
	{
		for (int c=0; c<3; c++)
		{
			rgbHist[c][i] = 0;
		}
	}
	for (int y=0; y<src.height(); y++)
	{
		for (int x=0; x<src.width(); x++)
		{
			QRgb pC = dst.pixel(x,y);
			rgbHist[0][qRed(pC)]++;	// Format_RGB32 -> subpixel should be bounded
			rgbHist[1][qGreen(pC)]++;
			rgbHist[2][qBlue(pC)]++;
		}
	}
	
	#define HIST_LO_THRES	(0.001)
	#define HIST_HI_THRES	(0.999)
	int rgbLUT[3][256];
	for (int c=0; c<3; c++)
	{
		int min, max;
		min = 0;
		max = 255;
		for (int i=1; i<256; i++)
		{
			rgbHist[c][i] += rgbHist[c][i-1];	// cumulative distribution
		}
		for (int i=0; i<256; i++)
		{
			if (rgbHist[c][i] <= (rgbHist[c][255]*HIST_LO_THRES)) min = i;
		}
		for (int i=255; i>=0; i--)
		{
			if (rgbHist[c][i] >= (rgbHist[c][255]*HIST_HI_THRES)) max = i;
		}
		for (int i=0; i<256; i++)
		{
			if (min == max)
			{
				rgbLUT[c][i] = i;
			}
			else
			{
				int val = std::round((i-min)*(255.0/(max - min)));
				rgbLUT[c][i] = qBound(0, val, 255);
			}
		}
	}
	#undef HIST_LO_THRES
	#undef HIST_HI_THRES
	
	unsigned long long int hist[256];
	for (int i=0; i<256; i++)
	{
		hist[i] = 0;
	}
	for (int y=0; y<src.height(); y++)
	{
		for (int x=0; x<src.width(); x++)
		{
			QRgb pC = dst.pixel(x,y);
			pC = qRgb(rgbLUT[0][qRed(pC)], rgbLUT[1][qGreen(pC)], rgbLUT[2][qBlue(pC)]);
			hist[qGray(pC)]++;
			dst.setPixel(x, y, pC);
		}
	}
	
	double mean = 0.0;
	double weight = 0.0;
	for (int i=1; i<256; i++)
	{
		mean += hist[i] * std::log(i/255.0);
		weight += hist[i];
	}
	mean /= weight;
	double gamma = -1.0/mean;

	int LUT[256];

	for (int i=0; i<256; i++)
	{
		if (!std::isfinite(gamma))
		{
			LUT[i] = i;
		}
		else
		{
			int val = std::round(255.0*std::pow(i/255.0, std::sqrt(gamma)));
			LUT[i] = qBound(0, val, 255);
		}
	}

	
	for (int y=0; y<src.height(); y++)
	{
		for (int x=0; x<src.width(); x++)
		{
			QRgb pC = dst.pixel(x,y);
			dst.setPixel(x, y, qRgb(LUT[qRed(pC)], LUT[qGreen(pC)], LUT[qBlue(pC)]));
		}
	}

	dst = dst.convertToFormat(src.format());
	if (src.hasAlphaChannel())
	{
		dst.setAlphaChannel(src.convertToFormat(QImage::Format_Alpha8));
	}
	return dst;
}

