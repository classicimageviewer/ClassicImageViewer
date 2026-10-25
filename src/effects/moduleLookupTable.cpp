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

#include "moduleLookupTable.h"
#include "globals.h"
#include "io/imageIO.h"
#include <QFile>
#include <QDebug>
#include <cmath>


EffectModuleLookupTable::EffectModuleLookupTable(QObject * parent) : QObject(parent)
{
	setModuleName("EffectModuleLookupTable");
}

EffectModuleLookupTable::~EffectModuleLookupTable()
{
	
}

QList<EffectBase::ParameterCluster> EffectModuleLookupTable::getListOfParameterClusters()
{
	QList<EffectBase::ParameterCluster> cluster;

	cluster += uiParamFilePath("", "lutFile", QString(), tr("Cube files (*.cube *.png);;All files (*.*)"));

	return cluster;
}

bool EffectModuleLookupTable::loadCubeFile(const QString & filepath)
{
	QFileInfo fi(filepath);
	if (!fi.exists() || !fi.isFile()) return false;

	if (fi.size() > 100LL * 1024 * 1024) return false;

	QFile file(filepath);
	if (!file.open(QFile::ReadOnly)) return false;

	QByteArray raw = file.readAll();
	file.close();
	
	// discard binary file
	for (auto c : raw)
	{
		if (c < '\t') return false;
	}

	QString content = QString::fromUtf8(raw);
	if (content.isEmpty()) return false;

	QStringList lines = content.split('\n');
	int lineIndex = 0;
	int totalLines = lines.size();

	bool sizeFound = false;
	while (lineIndex < totalLines)
	{
		QString line = lines[lineIndex].trimmed();
		lineIndex += 1;

		if (line.isEmpty() || line.startsWith('#'))
			continue;

		if (line.startsWith("LUT_3D_SIZE", Qt::CaseInsensitive))
		{
			QStringList parts = line.simplified().split(' ');
			if (parts.size() >= 2)
			{
				bool ok;
				int s = parts[1].toInt(&ok);
				if (ok && s > 0 && s <= 256)
				{
					cubeSize = s;
					sizeFound = true;
				}
			}
			break;
		}
	}

	if (!sizeFound || cubeSize <= 0)
	{
		// qDebug() << "CubeFile: size not found";
		return false;
	}

	// qDebug() << "CubeFile: size =" << cubeSize << "expected entries =" << cubeSize * cubeSize * cubeSize;

	int totalEntries = cubeSize * cubeSize * cubeSize;
	cube.resize(totalEntries);

	int count = 0;
	while (lineIndex < totalLines && count < totalEntries)
	{
		QString line = lines[lineIndex].trimmed();
		lineIndex += 1;

		if (line.isEmpty() || line.startsWith('#')) continue;

		QStringList triplet = line.simplified().split(' ');
		if (triplet.size() < 3)
		{
			// try CSV style
			triplet = line.simplified().split(',');
			if (triplet.size() < 3) continue;
		}

		bool ok;
		float r = triplet[0].toFloat(&ok);
		if (!ok) continue;
		float g = triplet[1].toFloat(&ok);
		if (!ok) continue;
		float b = triplet[2].toFloat(&ok);
		if (!ok) continue;

		cube[count].r = qBound(0.0f, r, 1.0f);
		cube[count].g = qBound(0.0f, g, 1.0f);
		cube[count].b = qBound(0.0f, b, 1.0f);
		count += 1;
	}

	// qDebug() << "CubeFile: parsed" << count << "entries";

	if (count < totalEntries)
	{
		// qDebug() << "CubeFile: incomplete";
		return false;
	}

	return true;
}

bool EffectModuleLookupTable::loadLutFromTexture(const QString & filepath)
{

	QFileInfo fi(filepath);
	if (!fi.exists() || !fi.isFile()) return false;

	// Load via ImageIO
	ImageIO imgIO;
	XImage ximg = imgIO.loadFile(filepath);
	if (ximg.images.isEmpty()) return false;

	QImage img = ximg.images.first().convertToFormat(QImage::Format_RGB32);
	if (img.isNull()) return false;

	int w = img.width();
	int h = img.height();
	int totalPixels = w * h;

	// cube size N^3 == pixel count
	int n = 0;
	for (int candidate = 2; candidate <= 256; candidate++)
	{
		if (candidate * candidate * candidate == totalPixels)
		{
			n = candidate;
			break;
		}
	}

	if (n == 0) return false;

	cubeSize = n;
	cube.resize(totalPixels);

	// square (N^3) or wide layout (N^2 x N)
	if ((w == h) || ((w == (n * n)) && (h == n)))
	{
		int i = 0;
		for (int y = 0; y < h; y++)
		{
			const QRgb * row = reinterpret_cast<const QRgb *>(img.constScanLine(y));
			for (int x = 0; x < w; x++)
			{
				cube[i].r = qRed(row[x])   / 255.0f;
				cube[i].g = qGreen(row[x]) / 255.0f;
				cube[i].b = qBlue(row[x])  / 255.0f;
				i += 1;
			}
		}
		return true;
	}
	
	return false;
}

EffectModuleLookupTable::rgbTriplet EffectModuleLookupTable::indexCube(int x, int y, int z)
{
	return cube[(z * cubeSize * cubeSize) + (y * cubeSize) + x];
}


QRgb EffectModuleLookupTable::cubeInterpolation(QRgb rgb, bool hasAlpha)
{
	float scale = cubeSize - 1;
	scale /= 255.0f;

	float xf = qRed(rgb) * scale;
	float yf = qGreen(rgb) * scale;
	float zf = qBlue(rgb) * scale;

	int xi = static_cast<int>(xf);
	int yi = static_cast<int>(yf);
	int zi = static_cast<int>(zf);

	xi = qBound(0, xi, cubeSize - 2);
	yi = qBound(0, yi, cubeSize - 2);
	zi = qBound(0, zi, cubeSize - 2);

	float fx = xf - xi;
	float fy = yf - yi;
	float fz = zf - zi;

	auto c000 = indexCube(  xi,     yi,     zi    );
	auto c111 = indexCube(  xi + 1, yi + 1, zi + 1);

	rgbTriplet interpolated;

	if (fx >= fy)
	{
		if (fy >= fz)
		{
			// fx >= fy >= fz
			auto c100 = indexCube(  xi + 1, yi,     zi    );
			auto c110 = indexCube(  xi + 1, yi + 1, zi    );
			interpolated = (1.0f - fx) * c000 + (fx - fy) * c100 + (fy - fz) * c110 + fz * c111;
		}
		else if (fx >= fz)
		{
			// fx >= fz >= fy
			auto c100 = indexCube(  xi + 1, yi,     zi    );
			auto c101 = indexCube(  xi + 1, yi,     zi + 1);
			interpolated = (1.0f - fx) * c000 + (fx - fz) * c100 + (fz - fy) * c101 + fy * c111;
		}
		else
		{
			// fz >= fx >= fy
			auto c001 = indexCube(  xi,     yi,     zi + 1);
			auto c101 = indexCube(  xi + 1, yi,     zi + 1);
			interpolated = (1.0f - fz) * c000 + (fz - fx) * c001 + (fx - fy) * c101 + fy * c111;
		}
	}
	else
	{
		if (fz >= fy)
		{
			// fz >= fy >= fx
			auto c001 = indexCube(  xi,     yi,     zi + 1);
			auto c011 = indexCube(  xi,     yi + 1, zi + 1);
			interpolated = (1.0f - fz) * c000 + (fz - fy) * c001 + (fy - fx) * c011 + fx * c111;
		}
		else if (fz >= fx)
		{
			// fy >= fz >= fx
			auto c010 = indexCube(  xi,     yi + 1, zi    );
			auto c011 = indexCube(  xi,     yi + 1, zi + 1);
			interpolated = (1.0f - fy) * c000 + (fy - fz) * c010 + (fz - fx) * c011 + fx * c111;
		}
		else
		{
			// fy >= fx >= fz
			auto c010 = indexCube(  xi,     yi + 1, zi    );
			auto c110 = indexCube(  xi + 1, yi + 1, zi    );
			interpolated = (1.0f - fy) * c000 + (fy - fx) * c010 + (fx - fz) * c110 + fz * c111;
		}
	}

	int rOut = qBound(0, static_cast<int>(std::round(interpolated.r * 255.0f)), 255);
	int gOut = qBound(0, static_cast<int>(std::round(interpolated.g * 255.0f)), 255);
	int bOut = qBound(0, static_cast<int>(std::round(interpolated.b * 255.0f)), 255);

	if (hasAlpha)
	{
		return qRgba(rOut, gOut, bOut, qAlpha(rgb));
	}
	else
	{
		return qRgb(rOut, gOut, bOut);
	}
}

QImage EffectModuleLookupTable::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	QString filepath = getParamStringValue(parameters, "lutFile", QString());

	if (filepath.isEmpty())
		return image;

	if (!loadCubeFile(filepath))
	{
		if (!loadLutFromTexture(filepath))
		{
			//qDebug() << "LUT not valid, filepath:" << filepath;
			return image;
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

	int w = dst.width();
	int h = dst.height();

	#pragma omp parallel for schedule(dynamic, 1)
	for (int y = 0; y < h; y++)
	{
		QRgb * row = reinterpret_cast<QRgb *>(dst.scanLine(y));
		for (int x = 0; x < w; x++)
		{
			row[x] = cubeInterpolation(row[x], hasAlpha);
		}
	}

	dst = dst.convertToFormat(image.format());
	return dst;
}

