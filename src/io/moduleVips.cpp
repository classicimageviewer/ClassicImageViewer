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


#if defined(HAS_VIPS)
#include <vips.h>
#include <vips/vips8>
#include <vips/version.h>
using namespace vips;
#endif

#include "moduleVips.h"
#include "globals.h"
#include <ctime>
#include <QDebug>
#include <QStandardPaths>
#include <QFile>


IOmoduleVips::IOmoduleVips(QObject * parent) : QObject(parent)
{
	formatsOfFits = QStringList();
	formatsOfHeif = QStringList();
	formatsOfNifti = QStringList();
	formatsOfJp2k = QStringList();
	formatsOfJxl = QStringList();
#if defined(HAS_VIPS)
	QString testFileBasePath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QString("/test_%1.").arg(time(NULL));
	QString testFilePath;
	VImage testVImg = VImage::black(16, 16, VImage::option()->set("bands",3));
	
	bool flag = false;
	#define TEST_VIPS(LIST, EXT, ...)	flag = false; \
						testFilePath = testFileBasePath + QString(#EXT); \
						try {\
							testVImg.EXT ## save(testFilePath.toUtf8().data()); \
							flag = true; \
						} \
						catch (vips::VError const&) {}\
						if (flag) \
						{ \
							flag = false; \
							try {\
								VImage readBack = VImage::EXT ## load(testFilePath.toUtf8().data()); \
								flag = true; \
							} \
							catch (vips::VError const&) {}\
							QFile file(testFilePath); \
							file.remove(); \
						} \
						if (flag) { \
							qDebug() << "File format '" << #EXT << "' available via libvips"; \
							QStringList newExts = {__VA_ARGS__}; \
							supportedInputFormats += newExts; \
							supportedOutputFormats += newExts; \
							LIST += newExts; \
						}
	
	TEST_VIPS(formatsOfFits, fits, "fits", "fit", "fts");
	TEST_VIPS(formatsOfHeif, heif, "heif", "heifs", "heic", "heics", "avci", "avcs", "avif", "hif");
	TEST_VIPS(formatsOfNifti, nifti, "nii");
	#if ((VIPS_MAJOR_VERSION >= 8) && (VIPS_MINOR_VERSION >= 11))
	TEST_VIPS(formatsOfJp2k, jp2k, "jp2", "j2k", "jpf", "jpm", "jpg2", "j2c", "jpc", "jpx", "mj2");
	TEST_VIPS(formatsOfJxl, jxl, "jxl");
	#endif
	
	#undef TEST_VIPS
#endif
}

IOmoduleVips::~IOmoduleVips()
{
}


QImage IOmoduleVips::loadFile(QString path)
{
#if defined(HAS_VIPS)
	VImage vImg;
	bool vImgLoaded = true;
	do {
		#define LOAD_VIPS(LIST, EXT)	if (LIST.size() > 0) \
						{ \
							bool success = true; \
							try {\
								vImg = VImage::EXT ## load(path.toUtf8().data()); \
							} \
							catch (vips::VError const&) { \
								success = false;\
							} \
							if (success) break; \
						}
		LOAD_VIPS(formatsOfFits, fits);
		LOAD_VIPS(formatsOfHeif, heif);
		LOAD_VIPS(formatsOfNifti, nifti);
		#if ((VIPS_MAJOR_VERSION >= 8) && (VIPS_MINOR_VERSION >= 11))
		LOAD_VIPS(formatsOfJp2k, jp2k);
		LOAD_VIPS(formatsOfJxl, jxl);
		#endif
		#undef LOAD_VIPS
		vImgLoaded = false;
	} while (0);
	if (vImgLoaded && (vImg.width() > 0) && (vImg.height() > 0))
	{
		if (vImg.format() != VIPS_FORMAT_UCHAR)
		{
			vImg = vImg.cast(VIPS_FORMAT_UCHAR, VImage::option()->set("shift", true));
		}
		vImg = vImg.copy(VImage::option()->set("coding", VIPS_CODING_NONE)->set("interpretation", VIPS_INTERPRETATION_RGB));
		
		if ((vImg.bands() == 4) && vImg.has_alpha())
		{
			QImage i = QImage(vImg.width(), vImg.height(), QImage::Format_ARGB32);
			memcpy(i.bits(), vImg.data(), vImg.width()*vImg.height()*4);
			return i;
		}
		else
		if (vImg.bands() == 3)
		{
			QImage i = QImage(vImg.width(), vImg.height(), QImage::Format_RGB888);
			memcpy(i.bits(), vImg.data(), vImg.width()*vImg.height()*3);
			return i.convertToFormat(QImage::Format_RGB32);
		}
		else
		{
			// can't
		}
	}
#else
	Q_UNUSED(path);
#endif
	
	// can't read
	return QImage();
}

QImage IOmoduleVips::loadThumbnail(QString path, QSize thumbnailSize)
{
	Q_UNUSED(path);
	Q_UNUSED(thumbnailSize);
	return QImage();
}

QList<IObase::ParameterCluster> IOmoduleVips::getListOfParameterClusters(QString format)
{
	QList<IObase::ParameterCluster> cluster = QList<IObase::ParameterCluster>();
	if (formatsOfHeif.contains(format) || formatsOfJp2k.contains(format) || formatsOfJxl.contains(format))
	{
		IObase::ParameterCluster elem;
		elem.displayName = QString(tr("Quality"));
		elem.controlType = QString("spinbox");
		elem.parameterName = QString("Quality");
		elem.parameterValue = Globals::prefs->fetchSpecificParameter("IOmoduleVips/"+format, "Quality", QVariant(100));
		elem.parameterMinValue = QVariant(0);
		elem.parameterMaxValue = QVariant(100);
		cluster.append(elem);
	}
	if (formatsOfHeif.contains(format) || formatsOfJp2k.contains(format) || formatsOfJxl.contains(format))
	{
		IObase::ParameterCluster elem;
		elem.displayName = QString(tr("Lossless"));
		elem.controlType = QString("checkbox");
		elem.parameterName = QString("Lossless");
		elem.parameterValue = Globals::prefs->fetchSpecificParameter("IOmoduleVips/"+format, "Lossless", QVariant(false));
		cluster.append(elem);
	}
	if (formatsOfHeif.contains(format))
	{
		IObase::ParameterCluster elem;
		elem.displayName = QString(tr("Compression"));
		elem.controlType = QString("combobox");
		elem.parameterName = QString("HeifCompression");
		elem.parameterValue = Globals::prefs->fetchSpecificParameter("IOmoduleVips/"+format, "HeifCompression", QVariant(0));
		QStringList list = QStringList();
		list.append(QString(tr("Auto")));
		list.append(QString(tr("AOM")));
		list.append(QString(tr("RAV1E")));
		list.append(QString(tr("SVT")));
		list.append(QString(tr("x265")));
		elem.parameterMinValue = QVariant(list);
		cluster.append(elem);
	}
	return cluster;
}

bool IOmoduleVips::saveFile(QString path, QString format, QImage image, QList<IObase::ParameterCluster> parameters)
{
	bool success = false;
#if defined(HAS_VIPS)
	QImage img;
	VImage vImg;
	if (image.hasAlphaChannel())
	{
		img = image.convertToFormat(QImage::Format_ARGB32);
		vImg = VImage::new_from_memory((void*)img.constBits(), img.width()*img.height()*4, img.width(), img.height(), 4, VIPS_FORMAT_UCHAR);
	}
	else
	{
		img = image.convertToFormat(QImage::Format_RGB888);
		vImg = VImage::new_from_memory((void*)img.constBits(), img.width()*img.height()*3, img.width(), img.height(), 3, VIPS_FORMAT_UCHAR);
	}
	
	int quality = 100;
	bool lossless = false;
	int compression = 0;
	bool saveParameters = false;
	for (IObase::ParameterCluster elem : parameters)
	{
		if (elem.parameterName == "SaveFileDialogSaveParameters")	// this should be the first element in the list
		{
			if (elem.parameterValue.toBool()) saveParameters = true;
			continue;
		}
		if (saveParameters)
		{
			Globals::prefs->storeSpecificParameter("IOmoduleVips/"+format, elem.parameterName, elem.parameterValue);
		}
		
		if (elem.parameterName == "Quality")
		{
			quality = elem.parameterValue.toInt();
		} else
		if (elem.parameterName == "Lossless")
		{
			lossless = elem.parameterValue.toBool();
		} else
		if (elem.parameterName == "HeifCompression")
		{
			compression = elem.parameterValue.toInt();
		} else
		{
			qDebug() << "Invalid parameter" << elem.parameterName;
		}
	}
	
	if (formatsOfFits.contains(format))
	{
		try {
			vImg.fitssave(path.toUtf8().data());
			success = true;
		}
		catch (vips::VError const&) {}
	} else
	if (formatsOfHeif.contains(format))
	{
		try {
			vImg.heifsave(path.toUtf8().data(), VImage::option()
								->set("Q", quality)
								->set("lossless", lossless)
								->set("compression", compression)  );
			success = true;
		}
		catch (vips::VError const&) {}
	} else
	if (formatsOfNifti.contains(format))
	{
		try {
			vImg.niftisave(path.toUtf8().data());
			success = true;
		}
		catch (vips::VError const&) {}
	} else
	#if ((VIPS_MAJOR_VERSION >= 8) && (VIPS_MINOR_VERSION >= 11))
	if (formatsOfJp2k.contains(format))
	{
		try {
			vImg.jp2ksave(path.toUtf8().data(), VImage::option()
								->set("Q", quality)
								->set("lossless", lossless)  );
			success = true;
		}
		catch (vips::VError const&) {}
	} else
	if (formatsOfJxl.contains(format))
	{
		try {
			vImg.jxlsave(path.toUtf8().data(), VImage::option()
								->set("Q", quality)
								->set("lossless", lossless)  );
			success = true;
		}
		catch (vips::VError const&) {}
	} else
	#endif
	{}
#else
	Q_UNUSED(path);
	Q_UNUSED(image);
	Q_UNUSED(parameters);
#endif
	return success;
}


