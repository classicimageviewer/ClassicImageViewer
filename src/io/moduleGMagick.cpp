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


#include "moduleGMagick.h"
#include <ctime>
#include <QDebug>
#include <QStandardPaths>
#include <QFile>

#if defined(HAS_GMAGICK)
#include <Magick++.h>
#endif

IOmoduleGMagick::IOmoduleGMagick(QObject * parent) : QObject(parent)
{
#if defined(HAS_GMAGICK)
	QStringList magickInputFormats = {
						"ART", "AVS", "CAL", "CALS", "CIN", "CUT", "DCM", "DIB", "ICODIB", "DPX", "EMF", "WMFWIN32",
						"FAX", "G3", "FITS", "FPX", "GIF", "GIF87", "GRAY", "GRAYA", "R", "C", "G", "M", "B", "Y", "O",
						"K", "HEIF", "HEIC", "HRZ", "CUR", "ICO", "ICON", "BIE", "JBG", "JBIG", "JNX", "J2C", "JP2",
						"JPC", "PGX", "JXL", "MAC", "MAT", "MIFF", "MPC", "MPR", "MPRI", "MSL", "MTV", "MVG", "OTB",
						"PALM", "PCD", "PCDS", "PCL", "DCX", "PCX", "PDB", "PCT", "PICT", "PIX", "P7", "PAM", "PBM",
						"PGM", "PNM", "PPM", "PSD", "PWP", "RGB", "RGBA", "RLA", "RLE", "SCT", "SFW", "SGI", "RAS",
						"SUN", "SVG", "SVGZ", "ICB", "TGA", "VDA", "VST", "BIGTIFF", "GROUP4RAW", "PTIF", "TIF", "TIFF",
						"TIM", "TOPOL", "UIL", "VICAR", "VIFF", "XV", "WBMP", "WEBP", "WMF", "WPG", "XBM", "XCF",
						"PICON", "PM", "XPM", "XWD", "3FR", "ARW", "CR2", "CRW", "DCR", "DNG", "ERF", "K25", "KDC",
						"MEF", "MRW", "NEF", "ORF", "PEF", "RAF", "SR2", "SRF", "X3F"
					};
	
	QStringList magickOutputExcludeFormats = {
						"GROUP4RAW", "PCL", "GRAY", "GRAYA", "R", "C", "G", "M", "B", "Y", "O", "K", "MPR", "MPRI", "MSL", "P7", "MVG"
					};
	
	QStringList magickOutputFormats = magickInputFormats;
	
	for (const QString &ext : magickOutputExcludeFormats)
	{
		magickOutputFormats.removeAll(ext);
	}
	
	for (const QString &ext : magickInputFormats)
	{
		try {
			Magick::CoderInfo info(ext.toUtf8().data());
			if (info.isReadable())
			{
				supportedInputFormats += ext.toLower();
			}
		}
		catch(Magick::Error &e) {}
	}
	
	for (const QString &ext : magickOutputFormats)
	{
		try {
			Magick::CoderInfo info(ext.toUtf8().data());
			if (info.isWritable())
			{
				supportedOutputFormats += ext.toLower();
			}
		}
		catch(Magick::Error &e) {}
	}
#endif
}

IOmoduleGMagick::~IOmoduleGMagick()
{
}


QImage IOmoduleGMagick::loadFile(QString path)
{
#if defined(HAS_GMAGICK)
	Magick::Image * mImg = new Magick::Image();
	try {
		mImg->quiet(true);
		mImg->read(path.toUtf8().data());
		QImage i;
		if (mImg->matte())
		{
			i = QImage(mImg->columns(),mImg->rows(), QImage::Format_ARGB32);
		}
		else
		{
			i = QImage(mImg->columns(),mImg->rows(), QImage::Format_RGB32);
		}
		mImg->write(0,0,i.width(), i.height(), "BGRA", Magick::CharPixel, (void*)i.bits());
		if (mImg->classType() == Magick::PseudoClass)
		{
			i = i.convertToFormat(QImage::Format_Indexed8);
		}
		
		delete mImg;
		return i;
	}
	catch(Magick::Error &e) {}
	delete mImg;
#else
	Q_UNUSED(path);
#endif
	
	// can't read
	return QImage();
}

QImage IOmoduleGMagick::loadThumbnail(QString path, QSize thumbnailSize)
{
	Q_UNUSED(path);
	Q_UNUSED(thumbnailSize);
	return QImage();
}

QList<IObase::ParameterCluster> IOmoduleGMagick::getListOfParameterClusters(QString format)
{
	Q_UNUSED(format);
	return QList<IObase::ParameterCluster>();
}

bool IOmoduleGMagick::saveFile(QString path, QString format, QImage image, QList<IObase::ParameterCluster> parameters)
{
	Q_UNUSED(parameters);
#if defined(HAS_GMAGICK)
	Magick::Image * mImg = new Magick::Image();
	mImg->quiet(true);
	mImg->magick(format.toUtf8().data());
	QImage src32;
	if (image.hasAlphaChannel())
	{
		src32 = image.convertToFormat(QImage::Format_ARGB32);
		mImg->read(src32.width(), src32.height(), "BGRA", Magick::CharPixel, (void*)src32.bits());
	}
	else
	{
		src32 = image.convertToFormat(QImage::Format_RGB32);
		mImg->read(src32.width(), src32.height(), "BGRP", Magick::CharPixel, (void*)src32.bits());
	}
	if (image.depth() < 24)
	{
		mImg->classType(Magick::PseudoClass);
	}
	mImg->write(path.toUtf8().data());
	delete mImg;
	return true;
#else
	Q_UNUSED(path);
	Q_UNUSED(format);
	Q_UNUSED(image);
#endif
	return false;
}


