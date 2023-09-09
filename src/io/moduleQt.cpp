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


#include "moduleQt.h"
#include "globals.h"
#include <QDebug>
#include <QImageWriter>
#include <QImageReader>

IOmoduleQt::IOmoduleQt(QObject * parent) : QObject(parent)
{
	QImageReader reader;
	QImageWriter writer;
	
	for (const QByteArray & item : reader.supportedImageFormats())
		supportedInputFormats.append(QString::fromUtf8(item).toLower());
	
	for (const QByteArray & item : writer.supportedImageFormats())
		supportedOutputFormats.append(QString::fromUtf8(item).toLower());
}

IOmoduleQt::~IOmoduleQt()
{
}

QImage IOmoduleQt::loadFile(QString path)
{
	QImageReader reader(path);
	return reader.read();
}

QImage IOmoduleQt::loadThumbnail(QString path, QSize thumbnailSize)
{
	QImageReader reader(path);
	if ((reader.format() == "jpg") || (reader.format() == "jpeg"))	// only jpg is faster
	{
		QSize size = reader.size();
		if (size.isValid())
		{
			reader.setScaledSize(size.scaled(thumbnailSize, Qt::KeepAspectRatio));
			return reader.read();
		}
	}
	return QImage();
}

QList<IObase::ParameterCluster> IOmoduleQt::getListOfParameterClusters(QString format)
{
	QList<IObase::ParameterCluster> cluster = QList<IObase::ParameterCluster>();
	QImageWriter writer("dummy."+format);
	if (writer.supportsOption(QImageIOHandler::CompressionRatio) && (format != "png"))
	{
		IObase::ParameterCluster elem;
		if ((format == "tif") || (format == "tiff"))
		{
			elem.displayName = QString(tr("Compression"));
			elem.controlType = QString("combobox");
			elem.parameterName = QString("TiffCompression");
			elem.parameterValue = Globals::prefs->fetchSpecificParameter("IOmoduleQt/"+format, "TiffCompression", QVariant(0));
			QStringList list = QStringList();
			list.append(QString(tr("no compression")));
			list.append(QString(tr("LZW compression")));
			elem.parameterMinValue = QVariant(list);
		}
		else
		{
			elem.displayName = QString(tr("Compression ratio"));
			elem.controlType = QString("spinbox");
			elem.parameterName = QString("CompressionRatio");
			elem.parameterValue = Globals::prefs->fetchSpecificParameter("IOmoduleQt/"+format, "CompressionRatio", QVariant(100));
			elem.parameterMinValue = QVariant(0);
			elem.parameterMaxValue = QVariant(100);
		}
		cluster.append(elem);
	}
	if (writer.supportsOption(QImageIOHandler::Quality))
	{
		IObase::ParameterCluster elem;
		if (format == "png")
		{
			elem.displayName = QString(tr("Compression level"));
			elem.controlType = QString("spinbox");
			elem.parameterName = QString("PngCompressionLevel");
			elem.parameterValue = Globals::prefs->fetchSpecificParameter("IOmoduleQt/"+format, "PngCompressionLevel", QVariant(4));
			elem.parameterMinValue = QVariant(0);
			elem.parameterMaxValue = QVariant(9);
		}
		else
		{
			elem.displayName = QString(tr("Quality"));
			elem.controlType = QString("spinbox");
			elem.parameterName = QString("Quality");
			elem.parameterValue = Globals::prefs->fetchSpecificParameter("IOmoduleQt/"+format, "Quality", QVariant(100));
			elem.parameterMinValue = QVariant(0);
			elem.parameterMaxValue = QVariant(100);
		}
		cluster.append(elem);
	}
	if (writer.supportsOption(QImageIOHandler::OptimizedWrite))
	{
		IObase::ParameterCluster elem;
		elem.displayName = QString(tr("Optimized write"));
		elem.controlType = QString("checkbox");
		elem.parameterName = QString("OptimizedWrite");
		elem.parameterValue = Globals::prefs->fetchSpecificParameter("IOmoduleQt/"+format, "OptimizedWrite", QVariant(false));
		cluster.append(elem);
	}
	if (writer.supportsOption(QImageIOHandler::ProgressiveScanWrite))
	{
		IObase::ParameterCluster elem;
		elem.displayName = QString(tr("Progressive scan"));
		elem.controlType = QString("checkbox");
		elem.parameterName = QString("ProgressiveScanWrite");
		elem.parameterValue = Globals::prefs->fetchSpecificParameter("IOmoduleQt/"+format, "ProgressiveScanWrite", QVariant(false));
		cluster.append(elem);
	}
	return cluster;
}

bool IOmoduleQt::saveFile(QString path, QString format, QImage image, QList<IObase::ParameterCluster> parameters)
{
	QImageWriter writer(path);
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
			Globals::prefs->storeSpecificParameter("IOmoduleQt/"+format, elem.parameterName, elem.parameterValue);
		}
		
		if (elem.parameterName == "CompressionRatio")
		{
			writer.setCompression(elem.parameterValue.toInt());
		} else
		if (elem.parameterName == "TiffCompression")
		{
			writer.setCompression(elem.parameterValue.toInt());
		} else
		if (elem.parameterName == "Quality")
		{
			writer.setQuality(elem.parameterValue.toInt());
		} else
		if (elem.parameterName == "PngCompressionLevel")
		{
			writer.setQuality(10*(9-elem.parameterValue.toInt()));
		} else
		if (elem.parameterName == "OptimizedWrite")
		{
			writer.setOptimizedWrite(elem.parameterValue.toBool());
		} else
		if (elem.parameterName == "ProgressiveScanWrite")
		{
			writer.setProgressiveScanWrite(elem.parameterValue.toBool());
		} else
		{
			qDebug() << "Invalid parameter" << elem.parameterName;
		}
	}
	return writer.write(image);
}


