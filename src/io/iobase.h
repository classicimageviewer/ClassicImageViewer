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


#ifndef IOBASE_H
#define IOBASE_H

#include <QObject>
#include <QVariant>
#include <QImage>
#include <QList>
#include <QStringList>

class IObase
{
protected:
	QStringList supportedInputFormats;
	QStringList supportedOutputFormats;
public:
	IObase() {
		supportedInputFormats = QStringList();
		supportedOutputFormats = QStringList();
	};
	QStringList getInputFormats() {
		return supportedInputFormats;
	};
	QStringList getOutputFormats() {
		return supportedOutputFormats;
	};
	bool outputFormatSupported(QString format) {
		return supportedOutputFormats.contains(format.toLower());
	};
	
	virtual ~IObase() {};
	
	virtual QImage loadFile(QString path) = 0;
	virtual QImage loadThumbnail(QString path, QSize thumbnailSize) = 0;
	
	typedef struct {
		QString displayName;
		QString controlType;
		QString parameterName;
		QVariant parameterValue;
		QVariant parameterMinValue;
		QVariant parameterMaxValue;
	} ParameterCluster;
	
	virtual QList<IObase::ParameterCluster> getListOfParameterClusters(QString format) = 0;
	virtual bool saveFile(QString path, QString format, QImage image, QList<IObase::ParameterCluster> parameters) = 0;
};

#endif //IOBASE_H
