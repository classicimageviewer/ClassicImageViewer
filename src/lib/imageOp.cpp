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

