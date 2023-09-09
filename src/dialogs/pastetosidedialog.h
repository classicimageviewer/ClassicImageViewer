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


#ifndef PASTETOSIDEDIALOG_H
#define PASTETOSIDEDIALOG_H

#include <QDialog>
#include <QImage>
#include "ui_pastetosidedialog.h"

class PasteToSideDialog : public QDialog
{
	Q_OBJECT
private:
	Ui_PasteToSideDialog ui;
public:
	PasteToSideDialog(QWidget * parent = NULL);
	~PasteToSideDialog();
	int getSide();
	void savePreferences();
};

#endif //PASTETOSIDEDIALOG_H
