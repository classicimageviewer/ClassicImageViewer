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


#ifndef IMAGEIO_H
#define IMAGEIO_H

#include <QObject>
#include <QImage>
#include <QList>
#include <QStringList>

#include "io/iobase.h"

class ImageIO : public QObject
{
	Q_OBJECT
private: // variables
	QList<IObase*> modules;
	QStringList supportedInputFormats;
	QStringList supportedOutputFormats;
public:
	ImageIO(QObject * parent = NULL);
	~ImageIO();
	QStringList getInputFormats();
	QStringList getOutputFormats();
	QImage loadFile(QString path);
	QImage loadThumbnail(QString path, QSize thumbnailSize);
	QList<IObase::ParameterCluster> getListOfParameterClusters(QString format);
	bool saveFile(QString path, QString format, QImage image, QList<IObase::ParameterCluster> parameters);
};

#endif //IMAGEIO_H
