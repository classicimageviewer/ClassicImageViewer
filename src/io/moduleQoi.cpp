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


#include "moduleQoi.h"
#include <cstdio>

#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
#include <QColorSpace>
#endif

#define QOI_IMPLEMENTATION
#include "io/xt/qoi.h"

IOmoduleQoi::IOmoduleQoi(QObject * parent) : QObject(parent)
{
	supportedInputFormats.append(QString("qoi"));
	supportedOutputFormats.append(QString("qoi"));
	
	reloadConfig();
}

IOmoduleQoi::~IOmoduleQoi()
{
}

QImage IOmoduleQoi::loadFile(QString path)
{
	QImage img = QImage();
	
	FILE *f = fopen(path.toUtf8().data(), "rb");
	if (f == NULL) return img;
	
	size_t size, bytesRead;
	void *pixels, *data;

	fseek(f, 0, SEEK_END);
	size = ftell(f);
	if ((size <= 14 /*QOI_HEADER_SIZE*/) || (fseek(f, 0, SEEK_SET) != 0))
	{
		fclose(f);
		return img;
	}
	
	// test magic
	char magic[5];
	if (fread(magic, 1, 4, f) != 4)
	{
		fclose(f);
		return img;
	}
	if (memcmp(magic, "qoif", 4) != 0)
	{
		fclose(f);
		return img;
	}
	if (fseek(f, 0, SEEK_SET) != 0)
	{
		fclose(f);
		return img;
	}

	data = malloc(size);
	if (!data)
	{
		fclose(f);
		return img;
	}

	bytesRead = fread(data, 1, size, f);
	fclose(f);
	qoi_desc desc;
	pixels = (bytesRead != size) ? NULL : qoi_decode(data, bytesRead, &desc, 0);
	free(data);
	
	if (pixels == NULL) return img;
	
	if ((desc.width > 0) && (desc.height > 0))
	{
		if (desc.channels == 4)
		{
			img = QImage(desc.width, desc.height, QImage::Format_ARGB32);
			memcpy(img.bits(), pixels, desc.width*desc.height * 4);
			img = img.rgbSwapped();
			if (desc.colorspace == 1)
			{
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
				img.setColorSpace(QColorSpace(QColorSpace::Primaries::SRgb, QColorSpace::TransferFunction::Linear));
				img.convertToColorSpace(QColorSpace(QColorSpace::Primaries::SRgb, QColorSpace::TransferFunction::SRgb));
#endif
			}
		} else
		if (desc.channels == 3)
		{
			img = QImage(desc.width, desc.height, QImage::Format_RGB888);
			memcpy(img.bits(), pixels, desc.width*desc.height * 3);
			img = img.convertToFormat(QImage::Format_RGB32);
			if (desc.colorspace == 1)
			{
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
				img.setColorSpace(QColorSpace(QColorSpace::Primaries::SRgb, QColorSpace::TransferFunction::Linear));
				img.convertToColorSpace(QColorSpace(QColorSpace::Primaries::SRgb, QColorSpace::TransferFunction::SRgb));
#endif
			}
		}
	}
	
	free(pixels);
	
	return img;
}

QImage IOmoduleQoi::loadThumbnail(QString path, QSize thumbnailSize)
{
	Q_UNUSED(path);
	Q_UNUSED(thumbnailSize);
	return QImage();
}

QList<IObase::ParameterCluster> IOmoduleQoi::getListOfParameterClusters(QString format)
{
	Q_UNUSED(format);
	return QList<IObase::ParameterCluster>();
}

bool IOmoduleQoi::saveFile(QString path, QString format, QImage image, QList<IObase::ParameterCluster> parameters)
{
	Q_UNUSED(format);
	Q_UNUSED(parameters);
	QImage imgOut;
	qoi_desc desc;
	desc.width = image.width();
	desc.height = image.height();
	desc.colorspace = 0;
	
	if (image.hasAlphaChannel())
	{
		imgOut = image.convertToFormat(QImage::Format_ARGB32).rgbSwapped();
		desc.channels = 4;
	}
	else
	{
		imgOut = image.convertToFormat(QImage::Format_RGB888);
		desc.channels = 3;
	}
	return (qoi_write(path.toUtf8().data(), imgOut.constBits(), &desc) != 0);
}


