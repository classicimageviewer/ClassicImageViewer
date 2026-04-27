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


#ifndef SEAMCARVINGDIALOG_H
#define SEAMCARVINGDIALOG_H

#include <QDialog>
#include <QImage>
#include "ui_seamcarvingdialog.h"

class SeamCarvingDialog : public QDialog
{
	Q_OBJECT
private:
	Ui_SeamCarvingDialog ui;
	QImage srcImg;
	int direction, reduction;
	double previewScale;
	void displayPreview();
private slots:
	void changeDirection(int v);
	void spinBoxChanged(int value);
	void sliderChanged(int value);
public:
	SeamCarvingDialog(QImage image, QWidget * parent = NULL);
	~SeamCarvingDialog();
	void savePreferences();
	QImage shrinkImage(QImage i, double scale = 1.0);

};

#endif //SEAMCARVINGDIALOG_H
