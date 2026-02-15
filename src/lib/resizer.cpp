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


#if defined(HAS_GMAGICK)
#include <Magick++.h>
#endif
#if defined(HAS_VIPS)
#include <vips.h>
#include <vips/vips8>
using namespace vips;
#endif

#include <QDebug>
#include "resizer.h"
#include "globals.h"
#include <cmath>
#include <cstdint>
#include <QImage>


QImage Resizer::Resize(const QImage src, QSize newSize, Algorithm algo)
{
	if (algo == NearestNeighbor)
	{
		return src.scaled(newSize, Qt::IgnoreAspectRatio, Qt::FastTransformation);
	}
	if (algo == Bilinear)
	{
		return src.scaled(newSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	}
#if defined(HAS_VIPS)
	if ((algo == BicubicCM) || (algo == BicubicMN) || (algo == Lanczos))
	{
		QImage src32;
		if (src.hasAlphaChannel())
		{
			src32 = src.convertToFormat(QImage::Format_ARGB32);
		}
		else
		{
			src32 = src.convertToFormat(QImage::Format_RGB32);
		}
		// new_from_memory does not take ownership of data
		VImage vImg = VImage::new_from_memory((void*)src32.constBits(), src32.width()*src32.height()*4, src32.width(), src32.height(), 4, VIPS_FORMAT_UCHAR);
		double hScale = newSize.width() / (double)src32.width();
		double vScale = newSize.height() / (double)src32.height();
		const char * kernel = "cubic";
		//if (algo == BicubicCM) kernel = "cubic";
		if (algo == BicubicMN) kernel = "mitchell";
		if (algo == Lanczos) kernel = "lanczos3";
		vImg= vImg.resize(hScale, VImage::option()->set("kernel", kernel)->set("vscale", vScale));

		QImage dst;
		if (src.hasAlphaChannel())
		{
			dst = QImage(vImg.width(), vImg.height(), QImage::Format_ARGB32);
		}
		else
		{
			dst = QImage(vImg.width(), vImg.height(), QImage::Format_RGB32);
		}
		if (dst.size() != newSize)
		{
			qDebug() << "inaccurate scaling";
		}
		memcpy(dst.bits(), vImg.data(), vImg.width()*vImg.height()*4);
		return dst;
	}
#elif defined(HAS_GMAGICK)
	if ((algo == Bicubic) || (algo == Lanczos))
	{
		Magick::Image * mImg = new Magick::Image();
		QImage src32;
		if (src.hasAlphaChannel())
		{
			src32 = src.convertToFormat(QImage::Format_ARGB32);
		}
		else
		{
			src32 = src.convertToFormat(QImage::Format_RGB32);
		}
		mImg->read(src32.width(), src32.height(), "BGRP", Magick::CharPixel, (const void*)src32.constBits());
		if (algo == Bicubic) mImg->filterType(Magick::CubicFilter);
		if (algo == Lanczos) mImg->filterType(Magick::LanczosFilter);
		mImg->resize(Magick::Geometry(newSize.width(), newSize.height()));
		QImage dst;
		if (src.hasAlphaChannel())
		{
			dst = QImage(mImg->columns(),mImg->rows(), QImage::Format_ARGB32);
		}
		else
		{
			dst = QImage(mImg->columns(),mImg->rows(), QImage::Format_RGB32);
		}
		mImg->write(0,0,dst.width(), dst.height(), "BGRA", Magick::CharPixel, (void*)dst.bits());
		delete mImg;
		return dst;
	}
#endif

	// fallback
	return src;
}


QStringList Resizer::getListOfAlgorithms()
{
	QStringList list;
	list.append(QString(tr("Nearest Neighbor")));
	list.append(QString(tr("Bilinear")));
#if defined(HAS_VIPS)
	list.append(QString(tr("Bicubic (Catmull-Rom)")));
	list.append(QString(tr("Bicubic (Mitchell-Netravali)")));
	list.append(QString(tr("Lanczos")));
#elif defined(HAS_GMAGICK)
	list.append(QString(tr("Bicubic")));
	list.append(QString(tr("Lanczos")));
#endif
	
	return list;
}
