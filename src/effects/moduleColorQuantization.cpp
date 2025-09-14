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


#include "moduleColorQuantization.h"
#include "globals.h"
#include <QDebug>


EffectModuleColorQuantization::EffectModuleColorQuantization(QObject * parent) : QObject(parent)
{
	
}

EffectModuleColorQuantization::~EffectModuleColorQuantization()
{
	
}

QList<EffectBase::ParameterCluster> EffectModuleColorQuantization::getListOfParameterClusters()
{
	EffectBase::ParameterCluster elem;
	QList<EffectBase::ParameterCluster> cluster = QList<EffectBase::ParameterCluster>();
	
	elem.displayName = QString(tr("Method"));
	elem.controlType = QString("combobox");
	elem.parameterName = QString("Method");
	elem.parameterDefaultValue = QVariant(2);
	elem.parameterValue = Globals::prefs->fetchSpecificParameter("EffectModuleColorQuantization", "Method", elem.parameterDefaultValue);
	QStringList list = QStringList();
	list.append(QString(tr("Grayscale")));
	list.append(QString(tr("Web-safe colors")));
	list.append(QString(tr("Sorted colors")));
	elem.parameterMinValue = QVariant(list);
	cluster.append(elem);
	
	elem.displayName = QString(tr("Colors"));
	elem.controlType = QString("slider");
	elem.parameterName = QString("Colors");
	elem.parameterDefaultValue = QVariant(16);
	elem.parameterValue = Globals::prefs->fetchSpecificParameter("EffectModuleColorQuantization", "Colors", elem.parameterDefaultValue);
	elem.parameterMinValue = QVariant(2);
	elem.parameterMaxValue = QVariant(256);
	cluster.append(elem);
	
	elem.displayName = QString(tr("Dithering"));
	elem.controlType = QString("combobox");
	elem.parameterName = QString("Dithering");
	elem.parameterDefaultValue = QVariant(1);
	elem.parameterValue = Globals::prefs->fetchSpecificParameter("EffectModuleColorQuantization", "Dithering", elem.parameterDefaultValue);
	list = QStringList();
	list.append(QString(tr("None")));
	list.append(QString("Floyd-Steinberg"));
	list.append(QString("Stucki"));
	list.append(QString("Sierra"));
	elem.parameterMinValue = QVariant(list);
	cluster.append(elem);
	
	return cluster;
}

void EffectModuleColorQuantization::saveEffectParameters(QList<EffectBase::ParameterCluster> parameters)
{
	for (const EffectBase::ParameterCluster & elem : parameters)
	{
		Globals::prefs->storeSpecificParameter("EffectModuleColorQuantization", elem.parameterName, elem.parameterValue);
	}
}

inline int EffectModuleColorQuantization::colorError(QRgb a, QRgb b)
{
	int redMean = (qRed(a) + qRed(b));
	int diff = (qRed(a) - qRed(b));
	int error = diff*diff*(1024+redMean);
	diff = (qGreen(a) - qGreen(b));
	error += diff*diff*2048;
	diff = (qBlue(a) - qBlue(b));
	error += diff*diff*(1534-redMean);
	return error;
}

void EffectModuleColorQuantization::webSafeGetPalette(QImage image, int colors)
{
	std::vector<std::pair<int64_t, QRgb>> webPalette;
	for (int r=0; r<6; r++)
	{
		for (int g=0; g<6; g++)
		{
			for (int b=0; b<6; b++)
			{
				webPalette.push_back(std::make_pair(0, qRgb(r*0x33, g*0x33, b*0x33)));
			}
		}
	}
	for (int y=0; y<image.height(); y++)
	{
		QRgb* row = reinterpret_cast<QRgb *>(image.scanLine(y));
		for (int x=0; x<image.width(); x++)
		{
			QRgb rgbQ = row[x];
			int minError = 0;
			int selectedColor = 0;
			for (int i=0; i<216; i++)
			{
				int error = colorError(webPalette[i].second, rgbQ);
				if ((i==0) || (error < minError))
				{
					minError = error;
					selectedColor = i;
				}
			}
			webPalette[selectedColor].first += 1;
		}
	}
	std::sort(webPalette.begin(), webPalette.end());
	paletteLength = 0;
	for (int i=215; i>=0; i--)
	{
		palette[paletteLength] = webPalette[i].second;
		paletteLength++;
		if (paletteLength >= colors) break;
	}
}

void EffectModuleColorQuantization::sortedGetPalette(QImage image, int colors)
{
	typedef struct
	{
		int64_t rSum, gSum, bSum;
	} colorSum_t;
	
	std::vector<std::pair<uint32_t, int>> * levels[3];
	std::vector<colorSum_t> rgbSums;
	static colorSum_t zeroColors = {};
	int entries = 8;
	int count = 0;
	for (int level=0; level<3; level++)
	{
		levels[level] = new std::vector<std::pair<uint32_t, int>>(entries);
		for (int i=0; i<entries; i++)
		{
			levels[level]->at(i).first = 0;
			levels[level]->at(i).second = count;
			rgbSums.push_back(zeroColors);
			count += 1;
		}
		entries *= 8;
	}
	
	for (int y=0; y<image.height(); y++)
	{
		QRgb* row = reinterpret_cast<QRgb *>(image.scanLine(y));
		for (int x=0; x<image.width(); x++)
		{
			QRgb rgbQ = row[x];
			for (int level=0; level<3; level++)
			{
				int pos = ((qRed(rgbQ) >> (7-level)) << ((level+1)*2)) | ((qGreen(rgbQ) >> (7-level)) << (level+1)) | ((qBlue(rgbQ) >> (7-level)) << 0);
				levels[level]->at(pos).first += 1;
				int index = levels[level]->at(pos).second;
				rgbSums[index].rSum += qRed(rgbQ);
				rgbSums[index].gSum += qGreen(rgbQ);
				rgbSums[index].bSum += qBlue(rgbQ);
			}
		}
	}
	
	paletteLength = 0;
	for (int level=0; level<3; level++)
	{
		std::sort(levels[level]->rbegin(), levels[level]->rend());
		for (auto e: *levels[level])
		{
			double count = e.first;
			int r = rgbSums[e.second].rSum / count;
			int g = rgbSums[e.second].gSum / count;
			int b = rgbSums[e.second].bSum / count;
			palette[paletteLength] = qRgb(r,g,b);
			paletteLength++;
			if (paletteLength >= colors) break;
		}
		if (paletteLength >= colors) break;
	}
	
	for (int level=0; level<3; level++)
	{
		delete levels[level];
	}
}


QImage EffectModuleColorQuantization::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	int method = 2;
	int colors = 2;
	int dithering = 1;
	
	for (const EffectBase::ParameterCluster & elem : parameters)
	{
		if (elem.parameterName == "Method")
		{
			method = elem.parameterValue.toInt();
		}
		else
		if (elem.parameterName == "Colors")
		{
			colors = elem.parameterValue.toInt();
		}
		else
		if (elem.parameterName == "Dithering")
		{
			dithering = elem.parameterValue.toInt();
		}
		else
		{
			qDebug() << "Invalid parameter" << elem.parameterName;
		}
	}
	
	QImage dst = image.convertToFormat(QImage::Format_RGB32).copy();
	
	switch (method)
	{
		case 0:	//grayscale
			for (int i = 0; i < colors; i++)
			{
				int g = qBound(0.0, i*(256.0 / (colors-1)), 255.0);
				palette[i] = qRgb(g, g, g);
			}
			paletteLength = colors;
			break;
		case 1:	//web-safe
			webSafeGetPalette(dst, colors);
			break;
		default:
		case 2:	//sorted
			sortedGetPalette(dst, colors);
			break;
	}
	
	if (dithering == 0)
	{
		#pragma omp parallel for schedule(dynamic, 1)
		for (int y=0; y<dst.height(); y++)
		{
			QRgb* row = reinterpret_cast<QRgb *>(dst.scanLine(y));
			for (int x=0; x<dst.width(); x++)
			{
				QRgb rgbQ = row[x];
				int minError = 0;
				QRgb paletteColor = 0;
				for (int i=0; i<paletteLength; i++)
				{
					int error = colorError(palette[i], rgbQ);
					if ((i==0) || (error < minError))
					{
						minError = error;
						paletteColor = palette[i];
					}
				}
				row[x] = paletteColor;
			}
		}
	}
	else
	{
		int height = dst.height();
		int width = dst.width();
		for (int y=0; y<height; y++)
		{
			QRgb* row = reinterpret_cast<QRgb *>(dst.scanLine(y));
			for (int x=0; x<width; x++)
			{
				QRgb rgbQ = row[x];
				int minError = 0;
				QRgb paletteColor = 0;
				for (int i=0; i<paletteLength; i++)
				{
					int error = colorError(palette[i], rgbQ);
					if ((i==0) || (error < minError))
					{
						minError = error;
						paletteColor = palette[i];
					}
				}
				int rError = qRed(rgbQ) - qRed(paletteColor);
				int gError = qGreen(rgbQ) - qGreen(paletteColor);
				int bError = qBlue(rgbQ) - qBlue(paletteColor);
				row[x] = paletteColor;
				switch (dithering)
				{
					#define CLAMP(V) if (V<0) V=0; if (V>255) V=255;
					#define PIXEL_D(MUL,DIV) do { \
									int r = qRed(rowD[xd]) + (rError*MUL)/DIV; \
									CLAMP(r); \
									int g = qGreen(rowD[xd]) + (gError*MUL)/DIV; \
									CLAMP(g); \
									int b = qBlue(rowD[xd]) + (bError*MUL)/DIV; \
									CLAMP(b); \
									rowD[xd] = qRgb(r,g,b); \
								} while(0)
					#define PIXEL_L(X,MUL,DIV) do { \
									int xd = x-X; \
									if (xd>=0) PIXEL_D(MUL,DIV);\
								} while(0)
					#define PIXEL_C(  MUL,DIV) do { \
									int xd = x; \
									PIXEL_D(MUL,DIV);\
								} while(0)
					#define PIXEL_R(X,MUL,DIV) do { \
									int xd = x+X; \
									if (xd<width) PIXEL_D(MUL,DIV);\
								} while(0)
					default:
					case 1:	//Floyd-Steinberg
						{
							QRgb* rowD = row;
							PIXEL_R(1,7,16);
							int yd = y+1;
							if (yd >= height) break;
							rowD = reinterpret_cast<QRgb *>(dst.scanLine(yd));
							PIXEL_L(1,3,16);
							PIXEL_C(  5,16);
							PIXEL_R(1,1,16);
						}
						break;
					case 2:	//Stucki
						{
							QRgb* rowD = row;
							PIXEL_R(1,7,42);
							PIXEL_R(2,5,42);
							int yd = y+1;
							if (yd >= height) break;
							rowD = reinterpret_cast<QRgb *>(dst.scanLine(yd));
							PIXEL_L(2,2,42);
							PIXEL_L(1,4,42);
							PIXEL_C(  8,42);
							PIXEL_R(1,4,42);
							PIXEL_R(2,2,42);
							yd += 1;
							if (yd >= height) break;
							rowD = reinterpret_cast<QRgb *>(dst.scanLine(yd));
							PIXEL_L(2,1,42);
							PIXEL_L(1,2,42);
							PIXEL_C(  4,42);
							PIXEL_R(1,2,42);
							PIXEL_R(2,1,42);
						}
						break;
					case 3:	//Sierra
						{
							QRgb* rowD = row;
							PIXEL_R(1,5,32);
							PIXEL_R(2,3,32);
							int yd = y+1;
							if (yd >= height) break;
							rowD = reinterpret_cast<QRgb *>(dst.scanLine(yd));
							PIXEL_L(2,2,32);
							PIXEL_L(1,4,32);
							PIXEL_C(  5,32);
							PIXEL_R(1,4,32);
							PIXEL_R(2,2,32);
							yd += 1;
							if (yd >= height) break;
							rowD = reinterpret_cast<QRgb *>(dst.scanLine(yd));
							PIXEL_L(1,2,32);
							PIXEL_C(  3,32);
							PIXEL_R(1,2,32);
						}
						break;
					#undef PIXEL_R
					#undef PIXEL_L
				}
			}
		}
	}
	
	if ((method == 0) && (colors == 2))	// black and white
	{
		dst = dst.convertToFormat(QImage::Format_Mono, Qt::MonoOnly | Qt::ThresholdDither | Qt::AvoidDither);
	}
	else
	{
	#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
		QVector<QRgb> colorTable(paletteLength);
	#else
		QList<QRgb> colorTable(paletteLength);
	#endif
		for (int i=0; i<paletteLength; i++)
		{
			colorTable[i] = palette[i];
		}
		dst = dst.convertToFormat(QImage::Format_Indexed8, colorTable, Qt::ThresholdDither | Qt::AvoidDither);
	}
	return dst;
}

