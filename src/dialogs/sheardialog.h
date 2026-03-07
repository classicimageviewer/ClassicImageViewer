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


#ifndef SHEARDIALOG_H
#define SHEARDIALOG_H

#include <QDialog>
#include <QImage>
#include "ui_sheardialog.h"

class ShearDialog : public QDialog
{
	Q_OBJECT
private:
	Ui_ShearDialog ui;
	QImage srcImg;
	int direction, fillMethod, shear;
	double blur, previewScale;
	QColor backgroundColor;
	void displaySheared();
private slots:
	void changeDirection(int v);
	void spinBoxChanged(int value);
	void spinBoxChanged(double value);
	void sliderChanged(int value);
	void changeFillMethod(int v);
	void changeBackgroundColor(bool b);
public:
	ShearDialog(QImage image, QWidget * parent = NULL);
	~ShearDialog();
	void savePreferences();
	QImage shearImage(QImage i, double scale = 1.0);

};

#endif //SHEARDIALOG_H
