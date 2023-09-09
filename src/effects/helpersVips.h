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


#ifndef HELPERSVIPS_H
#define HELPERSVIPS_H

#if defined(HAS_VIPS)
#include <vips.h>
#include <vips/vips8>
using namespace vips;
#endif

#include <QImage>


#if defined(HAS_VIPS)
static VImage convertQImageToVImage(QImage image)
{
	QImage src32;
	if (image.hasAlphaChannel())
	{
		src32 = image.convertToFormat(QImage::Format_ARGB32);
	}
	else
	{
		src32 = image.convertToFormat(QImage::Format_RGB32);
	}
	// new_from_memory does not take ownership of data
	VImage vImg = VImage::new_from_memory((void*)src32.constBits(), src32.width()*src32.height()*4, src32.width(), src32.height(), 4, VIPS_FORMAT_UCHAR);
	return vImg;
};

static QImage convertVImageToQImage(VImage image, bool hasAlpha)
{
	QImage dst;
	if (hasAlpha)
	{
		dst = QImage(image.width(), image.height(), QImage::Format_ARGB32);
	}
	else
	{
		dst = QImage(image.width(), image.height(), QImage::Format_RGB32);
	}
	memcpy(dst.bits(), image.data(), image.width()*image.height()*4);
	return dst;
};
#endif

#endif //HELPERSVIPS_H
