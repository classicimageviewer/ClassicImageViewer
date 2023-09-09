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


#ifndef HELPERSGM_H
#define HELPERSGM_H

#include <QImage>

#if defined(HAS_GMAGICK)
#include <Magick++.h>
#endif

#if defined(HAS_GMAGICK)
static Magick::Image * convertQImageToMImage(QImage image)
{
	Magick::Image * mImg = new Magick::Image();
	mImg->quiet(true);
	QImage src32;
	if (image.hasAlphaChannel())
	{
		src32 = image.convertToFormat(QImage::Format_ARGB32);
		mImg->read(src32.width(), src32.height(), "BGRA", Magick::CharPixel, (const void*)src32.constBits());
	}
	else
	{
		src32 = image.convertToFormat(QImage::Format_RGB32);
		mImg->read(src32.width(), src32.height(), "BGRP", Magick::CharPixel, (const void*)src32.constBits());
	}
	return mImg;
};

static QImage convertMImageToQImage(Magick::Image * image, bool deleteMImage = true)
{
	QImage dst;
	if (image->matte())
	{
		dst = QImage(image->columns(),image->rows(), QImage::Format_ARGB32);
	}
	else
	{
		dst = QImage(image->columns(),image->rows(), QImage::Format_RGB32);
	}
	image->write(0,0,dst.width(), dst.height(), "BGRA", Magick::CharPixel, (void*)dst.bits());
	if (deleteMImage)
	{
		delete image;
	}
	return dst;
};
#endif


#endif //HELPERSGM_H
