// Copyright (C) 2025 zhuvoy
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


#ifndef EXTTOOLCONFIGDIALOG_H
#define EXTTOOLCONFIGDIALOG_H

#include <QDialog>
#include "ui_exttoolconfigdialog.h"

class ExtToolConfigDialog : public QDialog
{
	Q_OBJECT
private:
	Ui_ExtToolConfigDialog ui;
private slots:
	void help(bool b);
public:
	ExtToolConfigDialog(QWidget * parent = NULL);
	~ExtToolConfigDialog();
	void savePreferences();
};

#endif //EXTTOOLCONFIGDIALOG_H
