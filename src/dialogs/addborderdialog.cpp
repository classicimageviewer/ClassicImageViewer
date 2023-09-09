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


#include "addborderdialog.h"
#include "globals.h"
#include <QDebug>
#include <QColorDialog>

AddBorderDialog::AddBorderDialog(QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);
	setFixedSize(size());
	
	backgroundColor = Globals::prefs->fetchSpecificParameter("AddBorderDialog", "backgroundColor", QColor(Qt::black)).value<QColor>();
	ui.spinBoxTop->setValue(Globals::prefs->fetchSpecificParameter("AddBorderDialog", "top", 0).toInt());
	ui.spinBoxRight->setValue(Globals::prefs->fetchSpecificParameter("AddBorderDialog", "right", 0).toInt());
	ui.spinBoxBottom->setValue(Globals::prefs->fetchSpecificParameter("AddBorderDialog", "bottom", 0).toInt());
	ui.spinBoxLeft->setValue(Globals::prefs->fetchSpecificParameter("AddBorderDialog", "left", 0).toInt());
	
	connect(ui.pushButtonColor, SIGNAL(clicked(bool)), this, SLOT(changeBackgroundColor(bool)));
}

AddBorderDialog::~AddBorderDialog()
{
	
}

void AddBorderDialog::changeBackgroundColor(bool b)
{
	Q_UNUSED(b);
	QColorDialog * d = new QColorDialog(backgroundColor);
	if (d->exec() == QDialog::Accepted)
	{
		backgroundColor = d->selectedColor();
	}
	delete d;
}

QImage AddBorderDialog::addBorder(QImage i)
{
	int top = ui.spinBoxTop->value();
	int right = ui.spinBoxRight->value();
	int bottom = ui.spinBoxBottom->value();
	int left = ui.spinBoxLeft->value();
	QImage dst(i.size()+QSize(left+right, top+bottom), i.format());
	dst.fill(backgroundColor);
	QPainter p(&dst);
	p.drawImage(left, top, i);
	p.end();
	return dst;
}

void AddBorderDialog::savePreferences()
{
	Globals::prefs->storeSpecificParameter("AddBorderDialog", "backgroundColor", backgroundColor);
	Globals::prefs->storeSpecificParameter("AddBorderDialog", "top", ui.spinBoxTop->value());
	Globals::prefs->storeSpecificParameter("AddBorderDialog", "right", ui.spinBoxRight->value());
	Globals::prefs->storeSpecificParameter("AddBorderDialog", "bottom", ui.spinBoxBottom->value());
	Globals::prefs->storeSpecificParameter("AddBorderDialog", "left", ui.spinBoxLeft->value());
}

