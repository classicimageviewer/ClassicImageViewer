// Copyright (C) 2026 zhuvoy
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


#ifndef ADDTEXTDIALOG_H
#define ADDTEXTDIALOG_H

#include <QDialog>
#include <QImage>
#include "ui_addtextdialog.h"

class AddTextDialog : public QDialog
{
	Q_OBJECT
private:
	Ui_AddTextDialog ui;
	QRect selection;
	int positionMode;
	int relativePosition;
	QColor fontColor;
	void setPositionMode(int mode);
	void addUserText(bool newLine, QString userText);
private slots:
	void changeFontColor(bool b);
public:
	AddTextDialog(QRect selection, QMap<QString, QVariant> intialConfig = QMap<QString, QVariant>(), QWidget * parent = NULL);
	~AddTextDialog();
	QImage addTextToImage(QImage img);
	void savePreferences();
	QMap<QString, QVariant> getConfig();
};

#endif //ADDTEXTDIALOG_H
