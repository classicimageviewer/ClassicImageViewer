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


#include "padtosizedialog.h"
#include "globals.h"
#include <QDebug>
#include <QColorDialog>

PadToSizeDialog::PadToSizeDialog(QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);
	setFixedSize(size());
	
	backgroundColor = Globals::prefs->fetchSpecificParameter("PadToSizeDialog", "backgroundColor", QColor(Qt::black)).value<QColor>();
	ui.spinBoxWidth->setValue(Globals::prefs->fetchSpecificParameter("PadToSizeDialog", "width", 0).toInt());
	ui.spinBoxHeight->setValue(Globals::prefs->fetchSpecificParameter("PadToSizeDialog", "height", 0).toInt());
	switch (Globals::prefs->fetchSpecificParameter("PadToSizeDialog", "mode", 0).toInt())
	{
		default:
		case 0:
			ui.radioButtonC->setChecked(true);
			break;
		case 1:
			ui.radioButtonLU->setChecked(true);
			break;
		case 2:
			ui.radioButtonRU->setChecked(true);
			break;
		case 3:
			ui.radioButtonLD->setChecked(true);
			break;
		case 4:
			ui.radioButtonRD->setChecked(true);
			break;
	}
	
	connect(ui.pushButtonColor, SIGNAL(clicked(bool)), this, SLOT(changeBackgroundColor(bool)));
}

PadToSizeDialog::~PadToSizeDialog()
{
	
}

void PadToSizeDialog::changeBackgroundColor(bool b)
{
	Q_UNUSED(b);
	QColorDialog * d = new QColorDialog(backgroundColor);
	if (d->exec() == QDialog::Accepted)
	{
		backgroundColor = d->selectedColor();
	}
	delete d;
}

QImage PadToSizeDialog::padToSize(QImage i)
{
	int w = ui.spinBoxWidth->value();
	int h = ui.spinBoxHeight->value();
	if ((w == 0) && (h == 0)) return i;
	
	int mode = 0;
	if (ui.radioButtonLU->isChecked()) mode = 1;
	if (ui.radioButtonRU->isChecked()) mode = 2;
	if (ui.radioButtonLD->isChecked()) mode = 3;
	if (ui.radioButtonRD->isChecked()) mode = 4;
	
	QImage dst(QSize(w, h), i.format());
	dst.fill(backgroundColor);
	QPainter p(&dst);
	switch (mode)
	{
		default:
		case 0:
			p.drawImage((w - i.width())/2, (h - i.height())/2, i);
			break;
		case 1:
			p.drawImage(w - i.width(), h - i.height(), i);
			break;
		case 2:
			p.drawImage(0, h - i.height(), i);
			break;
		case 3:
			p.drawImage(w - i.width(), 0, i);
			break;
		case 4:
			p.drawImage(0, 0, i);
			break;
	}
	p.end();
	return dst;
}

void PadToSizeDialog::savePreferences()
{
	Globals::prefs->storeSpecificParameter("PadToSizeDialog", "backgroundColor", backgroundColor);
	Globals::prefs->storeSpecificParameter("PadToSizeDialog", "width", ui.spinBoxWidth->value());
	Globals::prefs->storeSpecificParameter("PadToSizeDialog", "height", ui.spinBoxHeight->value());
	Globals::prefs->storeSpecificParameter("PadToSizeDialog", "mode", 0);
	if (ui.radioButtonLU->isChecked()) Globals::prefs->storeSpecificParameter("PadToSizeDialog", "mode", 1);
	if (ui.radioButtonRU->isChecked()) Globals::prefs->storeSpecificParameter("PadToSizeDialog", "mode", 2);
	if (ui.radioButtonLD->isChecked()) Globals::prefs->storeSpecificParameter("PadToSizeDialog", "mode", 3);
	if (ui.radioButtonRD->isChecked()) Globals::prefs->storeSpecificParameter("PadToSizeDialog", "mode", 4);
}

