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


#ifndef IMAGEOP_H
#define IMAGEOP_H

#include <QObject>
#include <QImage>

class ImageOp : public QObject
{
	Q_OBJECT
public:
	using QObject::QObject;
	
	static QImage Blur(const QImage image, double radius);
	static QImage Sharpen(const QImage src, double strength);
	
	enum EdgeHandling
	{
		Mirror,
		Extend
	};
	
	static QImage Kernel2D(const QImage image, EdgeHandling edgeHandling, int kernelWidth, int kernelHeight, const int * kernel, int div, int offset);
	static QImage Kernel2D(const QImage image, EdgeHandling edgeHandling, QSize kernelSize, const int * kernel, int div, int offset)
	{
		return Kernel2D(image, edgeHandling, kernelSize.width(), kernelSize.height(), kernel, div, offset);
	};
	
	static QImage RotateRight(const QImage image);
	static QImage RotateLeft(const QImage image);
	static QImage MirrorVertical(const QImage image);
	static QImage MirrorHorizontal(const QImage image);
	static QImage Grayscale(const QImage image);
	static QImage Negative(const QImage image);
};

#endif //IMAGEOP_H
