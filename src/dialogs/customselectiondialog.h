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


#ifndef CUSTOMSELECTIONDIALOG_H
#define CUSTOMSELECTIONDIALOG_H

#include <QDialog>
#include <QImage>
#include <QPushButton>
#include "ui_customselectiondialog.h"

class CustomSelectionDialog : public QDialog
{
	Q_OBJECT
private:
	Ui_CustomSelectionDialog ui;
	QSize imageSize;
	QRect inputSelection;
	QPushButton * ignoreButton;
private slots:
	void checkSelection();
	void selectAR(bool b);
	void selectARfromList(int i);
	void xChanged(int x);
	void yChanged(int y);
	void wChanged(int w);
	void hChanged(int h);
	void arChanged(double d);
	void ignored(bool b);
public:
	CustomSelectionDialog(QSize imageSize, QRect oldSelection, QWidget * parent = NULL);
	~CustomSelectionDialog();
	QRect getSelection();
	void savePreferences();
};

#endif //CUSTOMSELECTIONDIALOG_H
