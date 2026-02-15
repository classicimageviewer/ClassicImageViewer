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
#include "globals.h"
#include <cmath>
#include <QGraphicsBlurEffect>

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

