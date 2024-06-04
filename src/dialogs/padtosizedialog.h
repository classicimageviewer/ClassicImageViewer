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


#ifndef PADTOSIZEDIALOG_H
#define PADTOSIZEDIALOG_H

#include <QDialog>
#include <QImage>
#include "ui_padtosizedialog.h"

class PadToSizeDialog : public QDialog
{
	Q_OBJECT
private:
	Ui_PadToSizeDialog ui;
	QColor backgroundColor;
private slots:
	void changeBackgroundColor(bool b);
	void selectSize(bool b);
	void selectAR(bool b);
public:
	PadToSizeDialog(QWidget * parent = NULL);
	~PadToSizeDialog();
	QImage padToSize(QImage i);
	void savePreferences();
};

#endif //PADTOSIZEDIALOG_H
