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


#ifndef RESIZEDIALOG_H
#define RESIZEDIALOG_H

#include <QDialog>
#include "ui_resizedialog.h"
#include "modules/resizer.h"

class ResizeDialog : public QDialog
{
	Q_OBJECT
private:
	Ui_ResizeDialog ui;
	int w,h,ow,oh;
	double inputAR;
	void displayOutputRes();
private slots:
	void inputToPixelsChanged(bool b);
	void inputToPercentsChanged(bool b);
	void spinBoxPixWidthChanged(int value);
	void spinBoxPixHeightChanged(int value);
	void spinBoxPercWidthChanged(double value);
	void spinBoxPercHeightChanged(double value);
	void checkBoxARchanged(int i);
public:
	ResizeDialog(int width, int height, QWidget * parent = NULL);
	~ResizeDialog();
	QSize getNewResolution();
	Resizer::Algorithm getAlgorithm();
	void savePreferences();

};

#endif //RESIZEDIALOG_H
