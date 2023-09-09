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


#ifndef SLIDESHOWDIALOG_H
#define SLIDESHOWDIALOG_H

#include <QDialog>
#include "ui_slideshowdialog.h"

class SlideshowDialog : public QDialog
{
	Q_OBJECT
private:
	Ui_SlideshowDialog ui;
public:
	SlideshowDialog(QWidget * parent = NULL);
	~SlideshowDialog();
	int getIntervalMs();
	void savePreferences();
};

#endif //SLIDESHOWDIALOG_H
