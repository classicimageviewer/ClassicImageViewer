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


#include "imageOp.h"
#include <omp.h>
#include "globals.h"
#include <cmath>
#include <QGraphicsBlurEffect>
#include <QDebug>
#include <QElapsedTimer>

QImage ImageOp::Blur(const QImage image, double radius)
{
	QGraphicsBlurEffect *e = new QGraphicsBlurEffect();
	e->setBlurRadius(radius);
	e->setBlurHints(QGraphicsBlurEffect::QualityHint);
	QGraphicsScene scene;
	QGraphicsPixmapItem item;
	item.setPixmap(QPixmap::fromImage(image));
	item.setGraphicsEffect(e);
	scene.addItem(&item);
	QImage dst(image.size(), QImage::Format_ARGB32);
	dst.fill(Qt::transparent);
	QPainter ptr(&dst);
	scene.render(&ptr, QRectF(), QRectF(0, 0, image.width(), image.height()));
	
	return dst.convertToFormat(image.format()); 
}

QImage ImageOp::Sharpen(const QImage src, double strength)
{
	QImage dst = src.convertToFormat(QImage::Format_RGB32).copy();
	
	if (strength < 0.01) strength = 0.01;
	int antistrength = std::round(1.0/strength);
	int muldiv = std::round(4096.0/antistrength);
	for (int y=1; y<src.height()-1; y++)
	{
		for (int x=1; x<src.width()-1; x++)
		{
			QRgb pC = src.pixel(x,y);
			int red = qRed(pC)*(antistrength+8);
			int green = qGreen(pC)*(antistrength+8);
			int blue = qBlue(pC)*(antistrength+8);
			for (int ky=-1; ky<=1; ky++)
			{
				for (int kx=-1; kx<=1; kx++)
				{
					if ((kx==0) && (ky==0)) continue;
					pC = src.pixel(x+kx,y+ky);
					red -= qRed(pC);
					green -= qGreen(pC);
					blue -= qBlue(pC);
				}
			}
			
			dst.setPixel(x, y, qRgb(qBound(0, (red*muldiv)/4096, 255), qBound(0, (green*muldiv)/4096, 255), qBound(0, (blue*muldiv)/4096, 255)));
		}
	}
	dst = dst.convertToFormat(src.format());

	if (src.hasAlphaChannel())
	{
		dst.setAlphaChannel(src.convertToFormat(QImage::Format_Alpha8));
	}
	return dst;
}


QImage ImageOp::Kernel2D(const QImage image, EdgeHandling edgeHandling, int kernelWidth, int kernelHeight, const int * kernel, int div, int offset)
{
	if ((kernelWidth <= 0) || (kernelHeight <= 0) || (kernel == NULL))
	{
		return image;
	}
	if (((kernelWidth % 2) == 0) || ((kernelHeight % 2) == 0))	// must be odd
	{
		return image;
	}
	
	//QElapsedTimer perfTimer;
	//perfTimer.start();
	
	int dx = kernelWidth / 2;
	int dy = kernelHeight / 2;
	
	bool hasAlpha = image.hasAlphaChannel();
	QImage src, dst;
	
	if (hasAlpha)
	{
		src = image.convertToFormat(QImage::Format_ARGB32);
	}
	else
	{
		src = image.convertToFormat(QImage::Format_RGB32);
	}
	dst = src.copy();
	
	auto srcRows = new QRgb *[image.height() + 2*dy];
	for (int y = 0; y < (image.height() + 2*dy); y++)
	{
		int py = y - dy;
		if (py < 0)
		{
			switch(edgeHandling)
			{
				case Mirror:
					py = abs(py);
					break;
				case Extend:
					py = 0;
					break;
			}
		}
		if (py >= image.height())
		{
			switch(edgeHandling)
			{
				case Mirror:
					py = 2*image.height() - py - 1;
					break;
				case Extend:
					py = image.height() - 1;
					break;
			}
		}
		srcRows[y] = reinterpret_cast<QRgb *>(src.scanLine(py));
	}
	
	omp_set_num_threads(Globals::getThreadCount());
	#pragma omp parallel for schedule(static, 1)
	for (int y = 0; y < image.height(); y++)
	{
		QRgb* row = reinterpret_cast<QRgb *>(dst.scanLine(y));
		for (int x = 0; x < image.width(); x++)
		{
			int sumArgb[4] = {0};
			for (int ky = -dy; ky <= dy; ky++)
			{
				int py = y + ky + dy;
				for (int kx = -dx; kx <= dx; kx++)
				{
					int multiplier = kernel[kernelWidth*(ky+dy) + (kx+dx)];
					if (multiplier == 0) continue;
					int px = x + kx;
					if (px < 0)
					{
						switch(edgeHandling)
						{
							case Mirror:
								px = abs(px);
								break;
							case Extend:
								px = 0;
								break;
						}
					}
					if (px >= image.width())
					{
						switch(edgeHandling)
						{
							case Mirror:
								px = 2*image.width() - px - 1;
								break;
							case Extend:
								px = image.width() - 1;
								break;
						}
					}
					QRgb pixel = srcRows[py][px];	// #AARRGGBB
					for (int c=0; c<4; c++)
					{
						sumArgb[c] += (pixel&0xFF) * multiplier;
						pixel>>=8;
					}
				}
			}
			QRgb pixel = 0;
			for (int c=0; c<4; c++)
			{
				if (div > 1)
				{
					sumArgb[c] /= div;
				}
				sumArgb[c] += offset;
				sumArgb[c] = qBound(0, sumArgb[c], 255);
				pixel >>= 8;
				pixel |= (sumArgb[c] << 24);
			}
			if (!hasAlpha)
			{
				pixel |= 0xFF000000UL;
			}
			row[x] = pixel;
		}
	}
	
	delete [] srcRows;
	
	//qDebug() << perfTimer.elapsed();
	
	return dst.convertToFormat(image.format());
}

QImage ImageOp::RotateRight(const QImage image)
{
	return image.transformed(QTransform().rotate(90.0), Qt::SmoothTransformation);
}

QImage ImageOp::RotateLeft(const QImage image)
{
	return image.transformed(QTransform().rotate(-90.0), Qt::SmoothTransformation);
}

QImage ImageOp::Rotate180(const QImage image)
{
	return image.transformed(QTransform().rotate(180.0), Qt::SmoothTransformation);
}

QImage ImageOp::MirrorVertical(const QImage image)
{
#if QT_VERSION < QT_VERSION_CHECK(6,9,0)
	return image.mirrored(false, true);
#else
	return image.flipped(Qt::Vertical);
#endif
}

QImage ImageOp::MirrorHorizontal(const QImage image)
{
#if QT_VERSION < QT_VERSION_CHECK(6,9,0)
	return image.mirrored(true, false);
#else
	return image.flipped(Qt::Horizontal);
#endif
}

QImage ImageOp::Grayscale(const QImage image)
{
	QImage dst = image.convertToFormat(QImage::Format_Grayscale8).convertToFormat(QImage::Format_RGB32);
	if (image.hasAlphaChannel())
	{
		dst.setAlphaChannel(image.convertToFormat(QImage::Format_Alpha8));
	}
	return dst;
}

QImage ImageOp::Negative(const QImage image)
{
	QImage dst = image;
	dst.invertPixels();
	return dst;
}

QImage ImageOp::SeamCarvingHorizontal(const QImage image, int reduction, int exclusionStart, int exclusionStop)
{
	bool hasAlpha = image.hasAlphaChannel();
	QImage src;
	
	if (hasAlpha)
	{
		src = image.copy().convertToFormat(QImage::Format_ARGB32);
	}
	else
	{
		src = image.copy().convertToFormat(QImage::Format_RGB32);
	}
	
	int * map = new int[image.width()*image.height()];
	int * seam = new int[image.height()];
	
	omp_set_num_threads(Globals::getThreadCount());
	
	for (int r = 0; r < reduction; r++)
	{
		int width = src.width()-r;
		int height = src.height();
		if (width == 0) break;
		
		#pragma omp parallel for schedule(static, 1)
		for (int y = 1; y < height-1; y++)
		{
			const QRgb * rows[3];
			rows[0] = reinterpret_cast<const QRgb *>(src.constScanLine(y-1));
			rows[1] = reinterpret_cast<const QRgb *>(src.constScanLine(y));
			rows[2] = reinterpret_cast<const QRgb *>(src.constScanLine(y+1));
			int * mapLine = &map[y*width];
			for (int x = 1; x < width-1; x++)
			{
				int energy = 0;
				QRgb pixel00 = rows[1][x];
				int r = qRed(pixel00);
				int g = qGreen(pixel00);
				int b = qBlue(pixel00);
				
				for (int yy = 0; yy < 3; yy++)
				{
					const QRgb * row = rows[yy];
					for (int xx = x-1; xx <= x+1; xx++)
					{
						energy += abs(qRed(row[xx]) - r);
						energy += abs(qGreen(row[xx]) - g);
						energy += abs(qBlue(row[xx]) - b);
					}
				}
				
				if (hasAlpha)
				{
					energy *= qAlpha(pixel00);
				}
				mapLine[x] = energy;
			}
			mapLine[0] = mapLine[1];
			mapLine[width-1] = mapLine[width-2];
			
			for (int x = 0; x < width; x++)
			{
				if ((x >= exclusionStart) && (x <= exclusionStop))
				{
					mapLine[x] *= 999;
				}
			}
		}
		memcpy(&map[0*width], &map[1*width], sizeof(int)*width);
		memcpy(&map[(height-1)*width], &map[(height-2)*width], sizeof(int)*width);
		
		//#pragma omp parallel for schedule(static, 1)
		for (int y = 1; y < height; y++)
		{
			int yp = y - 1;
			int * rowp = &map[yp * width];
			int * row  = &map[y  * width];
			
			row[0] += qMin(rowp[0], rowp[1]);
			for (int x = 1; x < width-1; x++)
			{
				row[x] += qMin(qMin(rowp[x-1], rowp[x+1]), rowp[x]);
			}
			row[width-1] += qMin(rowp[width-1], rowp[width-2]);
		}
		
		int * lastLine = &map[width * (height - 1)];
		int minEnergy = lastLine[0];
		int minEnergyIndex = 0;
		for (int x = 0; x < width; x++)
		{
			if (lastLine[x] < minEnergy)
			{
				minEnergy = lastLine[x];
				minEnergyIndex = x;
			}
		}
		seam[height - 1] = minEnergyIndex;
		int averageSeam = 0;
		for (int y = height - 1; y > 0; y--)
		{
			int x = seam[y];
			int xp = x - 1;
			if (xp < 0) xp = 0;
			int xn = x + 1;
			if (xn >= width) xn = width - 1;
			int ep = map[width * (y - 1) + xp];
			int e  = map[width * (y - 1) + x];
			int en = map[width * (y - 1) + xn];
			if (ep < e)
			{
				seam[y-1] = (ep < en) ? xp : xn;
			}
			else
			{
				seam[y-1] = (en < e) ? xn : x;
			}
			averageSeam += x;
		}
		averageSeam += seam[0];
		averageSeam /= height;
		if ((exclusionStart >= 0) && (exclusionStop >= 0))
		{
			if (averageSeam <= exclusionStart)
			{
				exclusionStart -= 1;
				exclusionStop -= 1;
			}
			else
			if (averageSeam <= exclusionStop)
			{
				exclusionStop -= 1;
			}
		}
		
		#pragma omp parallel for schedule(static, 1)
		for (int y = 0; y < height; y++)
		{
			QRgb * srcRow  = reinterpret_cast<QRgb *>(src.scanLine(y));
			memmove(srcRow + seam[y], srcRow + seam[y] + 1, sizeof(QRgb) * (width - 1 - seam[y]));
		}
	}
	
	delete [] map;
	delete [] seam;
	
	return src.copy(src.rect().adjusted(0,0,-reduction,0)).convertToFormat(image.format());
}

QImage ImageOp::SeamCarvingVertical(const QImage image, int reduction, int exclusionStart, int exclusionStop)
{
	return RotateRight(SeamCarvingHorizontal(RotateLeft(image), reduction, exclusionStart, exclusionStop));
}

QImage ImageOp::AddText(const QImage image, QRect rect, QString text, int alignment, QFont font, QColor fontColor)
{
	QImage img = image;
	
	QPainter painter(&(img));
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	painter.setFont(font);
	painter.setPen(QPen(fontColor));
	painter.setRenderHint(QPainter::Antialiasing);
	rect = img.rect().intersected(rect);
	painter.drawText(rect, alignment, text);
	painter.end();
	
	return img;
}

