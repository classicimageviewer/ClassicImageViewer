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


#ifndef MODULEQOI_H
#define MODULEQOI_H

#include "iobase.h"

#include <QObject>
#include <QVariant>
#include <QImage>
#include <QList>
#include <QStringList>

class IOmoduleQoi : public QObject, public IObase
{
	Q_OBJECT
public:
	IOmoduleQoi(QObject * parent = NULL);
	~IOmoduleQoi() override;
	
	QString moduleName(void) override {return "QOI";};
	QImage loadFile(QString path) override;
	QImage loadThumbnail(QString path, QSize thumbnailSize) override;

	QList<IObase::ParameterCluster> getListOfParameterClusters(QString format) override;
	bool saveFile(QString path, QString format, QImage image, QList<IObase::ParameterCluster> parameters) override;
};

#endif //MODULEQOI_H
