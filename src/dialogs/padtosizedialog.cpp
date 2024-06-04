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
#include <cmath>

PadToSizeDialog::PadToSizeDialog(QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);
	setFixedSize(size());
	
	backgroundColor = Globals::prefs->fetchSpecificParameter("PadToSizeDialog", "backgroundColor", QColor(Qt::black)).value<QColor>();
	ui.spinBoxWidth->setValue(Globals::prefs->fetchSpecificParameter("PadToSizeDialog", "width", 0).toInt());
	ui.spinBoxHeight->setValue(Globals::prefs->fetchSpecificParameter("PadToSizeDialog", "height", 0).toInt());
	ui.doubleSpinBoxWidthRatio->setValue(Globals::prefs->fetchSpecificParameter("PadToSizeDialog", "widthRatio", 1.0).toDouble());
	ui.doubleSpinBoxHeightRatio->setValue(Globals::prefs->fetchSpecificParameter("PadToSizeDialog", "heightRatio", 1.0).toDouble());
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
	
	switch (Globals::prefs->fetchSpecificParameter("PadToSizeDialog", "method", 0).toInt())
	{
		default:
		case 0:
			ui.radioButtonNewSize->setChecked(true);
			selectSize(true);
			break;
		case 1:
			ui.radioButtonAR->setChecked(true);
			selectAR(true);
			break;
	}
	
	connect(ui.pushButtonColor, SIGNAL(clicked(bool)), this, SLOT(changeBackgroundColor(bool)));
	connect(ui.radioButtonNewSize, SIGNAL(clicked(bool)), this, SLOT(selectSize(bool)));
	connect(ui.radioButtonAR, SIGNAL(clicked(bool)), this, SLOT(selectAR(bool)));
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

void PadToSizeDialog::selectSize(bool b)
{
	Q_UNUSED(b);
	ui.spinBoxWidth->setEnabled(true);
	ui.spinBoxHeight->setEnabled(true);
	ui.doubleSpinBoxWidthRatio->setEnabled(false);
	ui.doubleSpinBoxHeightRatio->setEnabled(false);
	ui.radioButtonAR->setChecked(false);
}

void PadToSizeDialog::selectAR(bool b)
{
	Q_UNUSED(b);
	ui.spinBoxWidth->setEnabled(false);
	ui.spinBoxHeight->setEnabled(false);
	ui.doubleSpinBoxWidthRatio->setEnabled(true);
	ui.doubleSpinBoxHeightRatio->setEnabled(true);
	ui.radioButtonNewSize->setChecked(false);
}

QImage PadToSizeDialog::padToSize(QImage i)
{
	int w = ui.spinBoxWidth->value();
	int h = ui.spinBoxHeight->value();
	if (ui.radioButtonAR->isChecked())
	{
		w = i.width();
		h = i.height();
		if ((w == 0) && (h == 0)) return i;
		double sourceAR = (double)w / (double)h;
		double destinationAR = ui.doubleSpinBoxWidthRatio->value() / ui.doubleSpinBoxHeightRatio->value();
		if (destinationAR > sourceAR)
		{
			w = std::round(h * destinationAR);
		}
		else
		{
			h = std::round(w / destinationAR);
		}
	}
	
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
	Globals::prefs->storeSpecificParameter("PadToSizeDialog", "widthRatio", ui.doubleSpinBoxWidthRatio->value());
	Globals::prefs->storeSpecificParameter("PadToSizeDialog", "heightRatio", ui.doubleSpinBoxHeightRatio->value());
	Globals::prefs->storeSpecificParameter("PadToSizeDialog", "mode", 0);
	if (ui.radioButtonLU->isChecked()) Globals::prefs->storeSpecificParameter("PadToSizeDialog", "mode", 1);
	if (ui.radioButtonRU->isChecked()) Globals::prefs->storeSpecificParameter("PadToSizeDialog", "mode", 2);
	if (ui.radioButtonLD->isChecked()) Globals::prefs->storeSpecificParameter("PadToSizeDialog", "mode", 3);
	if (ui.radioButtonRD->isChecked()) Globals::prefs->storeSpecificParameter("PadToSizeDialog", "mode", 4);
	Globals::prefs->storeSpecificParameter("PadToSizeDialog", "method", (ui.radioButtonAR->isChecked() ? 1:0));
}

