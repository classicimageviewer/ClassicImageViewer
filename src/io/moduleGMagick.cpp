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
	QStringList magickInputFormats = {	"ART", "AVIF", "AVS", "CALS", "CIN", "CGM", "CUR", "CUT", 
						"DCM", "DCX", "DIB", "DPX", "FAX", "FITS", "HEIF", "HPGL",
						"ICO", "JBIG", "JNG", "JP2", "JPC", "JXL", "MIFF", "MONO",
						"MNG", "MTV", "OTB", "PALM", "PAM", "PBM", "PCX", "PDB",
						"PGM", "PICON", "PICT", "PIX", "RLA", "RLE", "SCT", "SFW",
						"SGI", "SUN", "TGA", "TIFF", "TIM", "VICAR", "VIFF", "WBMP",
						"WPG", "XBM", "XCF", "XPM", "XWD" };
	for (const QString &ext : magickInputFormats)
	{
		try {
			Magick::CoderInfo info(ext.toUtf8().data());
			if (info.isReadable())
			{
				//TODO
				qDebug() << "File format '" << ext.toLower().toUtf8().data() << "' available via GraphicsMagick";
				supportedInputFormats += ext.toLower();
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
	Q_UNUSED(path);
	Q_UNUSED(format);
	Q_UNUSED(image);
	Q_UNUSED(parameters);
	return false;
}


