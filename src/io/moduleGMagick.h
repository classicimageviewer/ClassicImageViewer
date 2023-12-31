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


#ifndef MODULEGMAGICK_H
#define MODULEGMAGICK_H

#include "iobase.h"

#include <QObject>
#include <QVariant>
#include <QImage>
#include <QList>
#include <QStringList>

class IOmoduleGMagick : public QObject, public IObase
{
	Q_OBJECT
public:
	IOmoduleGMagick(QObject * parent = NULL);
	~IOmoduleGMagick() override;
	
	QImage loadFile(QString path) override;
	QImage loadThumbnail(QString path, QSize thumbnailSize) override;

	QList<IObase::ParameterCluster> getListOfParameterClusters(QString format) override;
	bool saveFile(QString path, QString format, QImage image, QList<IObase::ParameterCluster> parameters) override;
};

#endif //MODULEGMAGICK_H
