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


#ifndef SAVEFILEDIALOG_H
#define SAVEFILEDIALOG_H

#include <QFileDialog>
#include <QVBoxLayout>
#include "io/imageIO.h"

class SaveFileDialog : public QFileDialog
{
	Q_OBJECT
private: // typedefs
private: // variables
	QVBoxLayout * sidekickLayout;
	QString baseName;
	QString defaultExtension;
	ImageIO * imageIO;
private: // functions
	void deleteSideKick();
private slots:
	void filterChanged(const QString &filter);
public: // typedefs
	typedef struct
	{
		QString filePath;
		QString selectedFormat;
		QList<IObase::ParameterCluster> parameters;
	} ReturnCluster;
public:
	SaveFileDialog(const QString &caption, const QString &directory, const QString &filter, const QString &fileName, ImageIO * imageIO, QWidget *parent = NULL);
	~SaveFileDialog();
	
	ReturnCluster getReturnCluster();
	void saveSelectedExtension();
signals:
};


#endif //SAVEFILEDIALOG_H
