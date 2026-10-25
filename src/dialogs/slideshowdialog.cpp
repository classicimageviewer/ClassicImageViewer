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


#include "slideshowdialog.h"
#include "globals.h"
#include <cmath>
#include <QDebug>

SlideshowDialog::SlideshowDialog(QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);
	setFixedSize(size());
	
	ui.doubleSpinBoxInterval->setValue(Globals::prefs->fetchSpecificParameter("SlideshowDialog", "interval", QVariant(1.0)).toDouble());
	ui.comboBoxDirection->setCurrentIndex(Globals::prefs->fetchSpecificParameter("SlideshowDialog", "direction", QVariant(0)).toInt());
}

SlideshowDialog::~SlideshowDialog()
{
	
}

int SlideshowDialog::getIntervalMs()
{
	int interval = ui.doubleSpinBoxInterval->value() * 1000.0;
	return interval;
}

int SlideshowDialog::getDirection()
{
	switch(ui.comboBoxDirection->currentIndex())
	{
		default:
		case 0:
			return 1;
		case 1:
			return -1;
		case 2:
			return 0;
	}
}

void SlideshowDialog::savePreferences()
{
	Globals::prefs->storeSpecificParameter("SlideshowDialog", "interval", ui.doubleSpinBoxInterval->value());
	Globals::prefs->storeSpecificParameter("SlideshowDialog", "direction", ui.comboBoxDirection->currentIndex());
}

