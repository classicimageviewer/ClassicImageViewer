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


#include "pastetosidedialog.h"
#include "globals.h"
#include <QDebug>
#include <QColorDialog>

PasteToSideDialog::PasteToSideDialog(QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);
	setFixedSize(size());

	ui.radioButtonRight->setChecked(true);
	ui.radioButtonTop->setChecked(Globals::prefs->fetchSpecificParameter("PasteToSideDialog", "side", 0).toInt() == 1);
	ui.radioButtonRight->setChecked(Globals::prefs->fetchSpecificParameter("PasteToSideDialog", "side", 0).toInt() == 2);
	ui.radioButtonBottom->setChecked(Globals::prefs->fetchSpecificParameter("PasteToSideDialog", "side", 0).toInt() == 3);
	ui.radioButtonLeft->setChecked(Globals::prefs->fetchSpecificParameter("PasteToSideDialog", "side", 0).toInt() == 4);
}

PasteToSideDialog::~PasteToSideDialog()
{
	
}

int PasteToSideDialog::getSide()
{
	if (ui.radioButtonTop->isChecked()) return 1;
	if (ui.radioButtonRight->isChecked()) return 2;
	if (ui.radioButtonBottom->isChecked()) return 3;
	if (ui.radioButtonLeft->isChecked()) return 4;
	return 0;
}

void PasteToSideDialog::savePreferences()
{
	Globals::prefs->storeSpecificParameter("PasteToSideDialog", "side", getSide());
}

