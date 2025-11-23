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


#include <QDebug>
#include "imageIO.h"
#include "globals.h"

#include "io/moduleQt.h"
#include "io/moduleVips.h"
#include "io/moduleGMagick.h"
#include "io/moduleQoi.h"

bool ImageIO::debugPrintSupportedFormats = true;

ImageIO::ImageIO(QObject * parent) : QObject(parent)
{
	modules = QList<IObase*>();
	
	modules.append(new IOmoduleQt());
	modules.append(new IOmoduleVips());
	modules.append(new IOmoduleGMagick());
	modules.append(new IOmoduleQoi());
	
	for (IObase* item : modules)
	{
		if (debugPrintSupportedFormats)
		{
			qDebug() << "IO module " << item->moduleName() << " supported formats: " << item->getInputFormats().join(", ");
		}
		item->addInputFormatAlternatives("jpg", {"jpg", "jpeg", "jpe", "jif", "jfif", "jfi", "pjpeg", "pjp"});
		item->addInputFormatAlternatives("jpeg", {"jpg", "jpeg", "jpe", "jif", "jfif", "jfi", "pjpeg", "pjp"});
		item->addInputFormatAlternatives("heif", {"heif", "heifs", "heic", "heics", "avci", "avcs", "hif"});
		item->addInputFormatAlternatives("heic", {"heif", "heifs", "heic", "heics", "avci", "avcs", "hif"});
		item->addInputFormatAlternatives("fits", {"fits", "fit", "fts"});
	}
	debugPrintSupportedFormats = false;
	
	reloadConfig();
}

ImageIO::~ImageIO()
{
	for (IObase* item : modules)
	{
		delete item;
	}
}

QStringList ImageIO::getInputFormats()
{
	return supportedInputFormats;
}

QStringList ImageIO::getOutputFormats()
{
	return supportedOutputFormats;
}

QString ImageIO::extensionOf(QString path)
{
	int dotPos = path.lastIndexOf(".");
	if (dotPos >= 0)
	{
		return path.mid(dotPos+1).toLower();
	}
	return QString();
}

QImage ImageIO::loadFile(QString path)
{
	for (IObase* item : modules)
	{
		if (item->getInputFormats().contains(extensionOf(path)) || item->tryToOpenAll())
		{
			QImage i = item->loadFile(path);
			if (!(i.isNull())) 
			{
				//qDebug() << "Loader: " << item->moduleName();
				return i;
			}
		}
	}
	
	// can't read
	return QImage();
}

QImage ImageIO::loadThumbnail(QString path, QSize thumbnailSize)
{
	for (IObase* item : modules)
	{
		if (item->getInputFormats().contains(extensionOf(path)) && !(item->getBlockedThumbnailFormats().contains(extensionOf(path))))
		{
			QImage i = item->loadThumbnail(path, thumbnailSize);
			if (!(i.isNull())) return i;
		}
	}
	
	// try fullscale loader
	for (IObase* item : modules)
	{
		if (item->getInputFormats().contains(extensionOf(path)) || item->tryToOpenAll())
		{
			QImage i = item->loadFile(path);
			if (!(i.isNull())) return i.scaled(thumbnailSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		}
	}
	
	// can't read
	return QImage();
}

QList<IObase::ParameterCluster> ImageIO::getListOfParameterClusters(QString format)
{
	for (IObase* item : modules)
	{
		if (item->outputFormatSupported(format))
		{
			return item->getListOfParameterClusters(format);
		}
	}
	return QList<IObase::ParameterCluster>();
}

bool ImageIO::saveFile(QString path, QString format, QImage image, QList<IObase::ParameterCluster> parameters)
{
	for (IObase* item : modules)
	{
		if (item->outputFormatSupported(format))
		{
			return item->saveFile(path, format, image, parameters);
		}
	}
	return false;
}

QList<IObase*> ImageIO::getModules()
{
	return modules;
}

void ImageIO::reloadConfig()
{
	for (IObase* item : modules)
	{
		item->reloadConfig();
	}
	
	supportedInputFormats = QStringList();
	supportedOutputFormats = QStringList();
	
	for (IObase* item : modules)
	{
		supportedInputFormats += item->getInputFormats();
		supportedOutputFormats += item->getOutputFormats();
	}
	
	supportedInputFormats.removeDuplicates();
	supportedOutputFormats.removeDuplicates();
	supportedInputFormats.sort();
	supportedOutputFormats.sort();
}

