// Copyright (C) 2025 zhuvoy
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


#include "customselectiondialog.h"
#include "globals.h"
#include <QDebug>
#include <cmath>

CustomSelectionDialog::CustomSelectionDialog(QSize imageSize, QRect oldSelection, QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);
	setFixedSize(size());
	
	this->imageSize = imageSize;
	inputSelection = oldSelection;
	
	ui.spinBoxX->setMinimum(0);
	ui.spinBoxY->setMinimum(0);
	ui.spinBoxX->setMaximum(imageSize.width() - 1);
	ui.spinBoxY->setMaximum(imageSize.height() - 1);
	ui.spinBoxWidth->setMinimum(1);
	ui.spinBoxHeight->setMinimum(1);
	ui.spinBoxWidth->setMaximum(imageSize.width());
	ui.spinBoxHeight->setMaximum(imageSize.height());
	
	
	ui.comboBoxAR->addItem(QString(tr("Current image")));
	ui.comboBoxAR->addItem(QString("1:1"));
	ui.comboBoxAR->addItem(QString("5:4"));
	ui.comboBoxAR->addItem(QString("4:3"));
	ui.comboBoxAR->addItem(QString("3:2"));
	ui.comboBoxAR->addItem(QString("14:9"));
	ui.comboBoxAR->addItem(QString("16:10"));
	ui.comboBoxAR->addItem(QString("1.66:1"));
	ui.comboBoxAR->addItem(QString("16:9"));
	ui.comboBoxAR->addItem(QString("1.85:1"));
	ui.comboBoxAR->addItem(QString("2:1"));
	ui.comboBoxAR->addItem(QString("21:9"));
	ui.comboBoxAR->addItem(QString("2.35:1"));
	ui.comboBoxAR->addItem(QString("2.39:1"));
	ui.comboBoxAR->addItem(QString("9:16"));
	ui.comboBoxAR->addItem(QString("9:18"));
	ui.comboBoxAR->addItem(QString("9:19.5"));
	ui.comboBoxAR->addItem(QString("9:20"));
	
	ignoreButton = ui.buttonBox->addButton(QString(tr("Ignore")), QDialogButtonBox::ResetRole);
	ignoreButton->setVisible(false);
	connect(ignoreButton, SIGNAL(clicked(bool)), this, SLOT(ignored(bool)));
	
	connect(ui.comboBoxAR, SIGNAL(activated(int)), this, SLOT(selectARfromList(int)));
	connect(ui.checkBoxAR, SIGNAL(clicked(bool)), this, SLOT(selectAR(bool)));
	
	connect(ui.spinBoxX, SIGNAL(valueChanged(int)), this, SLOT(xChanged(int)));
	connect(ui.spinBoxY, SIGNAL(valueChanged(int)), this, SLOT(yChanged(int)));
	connect(ui.spinBoxWidth, SIGNAL(valueChanged(int)), this, SLOT(wChanged(int)));
	connect(ui.spinBoxHeight, SIGNAL(valueChanged(int)), this, SLOT(hChanged(int)));
	connect(ui.doubleSpinBoxWidthRatio, SIGNAL(valueChanged(double)), this, SLOT(arChanged(double)));
	connect(ui.doubleSpinBoxHeightRatio, SIGNAL(valueChanged(double)), this, SLOT(arChanged(double)));
	
	
	if (inputSelection.isNull())
	{
		ui.spinBoxX->setValue(Globals::prefs->fetchSpecificParameter("CustomSelectionDialog", "x", 0).toInt());
		ui.spinBoxY->setValue(Globals::prefs->fetchSpecificParameter("CustomSelectionDialog", "y", 0).toInt());
		ui.spinBoxWidth->setValue(Globals::prefs->fetchSpecificParameter("CustomSelectionDialog", "width", 0).toInt());
		ui.spinBoxHeight->setValue(Globals::prefs->fetchSpecificParameter("CustomSelectionDialog", "height", 0).toInt());
		ui.doubleSpinBoxWidthRatio->setValue(Globals::prefs->fetchSpecificParameter("CustomSelectionDialog", "widthRatio", 1.0).toDouble());
		ui.doubleSpinBoxHeightRatio->setValue(Globals::prefs->fetchSpecificParameter("CustomSelectionDialog", "heightRatio", 1.0).toDouble());
	}
	else
	{
		ui.spinBoxX->setValue(inputSelection.x());
		ui.spinBoxY->setValue(inputSelection.y());
		ui.spinBoxWidth->setValue(inputSelection.width());
		ui.spinBoxHeight->setValue(inputSelection.height());
		ui.doubleSpinBoxWidthRatio->setValue(static_cast<double>(inputSelection.width()) / inputSelection.height());
		ui.doubleSpinBoxHeightRatio->setValue(1.0);
	}
	ui.checkBoxAR->setChecked(Globals::prefs->fetchSpecificParameter("CustomSelectionDialog", "lockAR", QVariant(true)).toBool());
	
	ui.comboBoxAR->setCurrentIndex(0);
	selectAR(false);
}

CustomSelectionDialog::~CustomSelectionDialog()
{
	
}

void CustomSelectionDialog::selectAR(bool b)
{
	Q_UNUSED(b);
	bool lockAR = ui.checkBoxAR->isChecked();
	ui.doubleSpinBoxWidthRatio->setEnabled(lockAR);
	ui.doubleSpinBoxHeightRatio->setEnabled(lockAR);
	ui.comboBoxAR->setEnabled(lockAR);
	if (lockAR)
	{
		wChanged(ui.spinBoxWidth->value());
	}
}

void CustomSelectionDialog::arChanged(double d)
{
	Q_UNUSED(d);
	if (ui.checkBoxAR->isChecked())
	{
		wChanged(ui.spinBoxWidth->value());
	}
}

void CustomSelectionDialog::xChanged(int x)
{
	Q_UNUSED(x);
	checkSelection();
}

void CustomSelectionDialog::yChanged(int y)
{
	Q_UNUSED(y);
	checkSelection();
}

void CustomSelectionDialog::wChanged(int w)
{
	if (ui.checkBoxAR->isChecked())
	{
		int h = std::round(( ui.doubleSpinBoxHeightRatio->value() / ui.doubleSpinBoxWidthRatio->value() ) * w);
		if (h == 0) h = 1;
		ui.spinBoxHeight->blockSignals(true);
		ui.spinBoxHeight->setValue(h);
		ui.spinBoxHeight->blockSignals(false);
	}
	checkSelection();
}

void CustomSelectionDialog::hChanged(int h)
{
	if (ui.checkBoxAR->isChecked())
	{
		int w = std::round(( ui.doubleSpinBoxWidthRatio->value() / ui.doubleSpinBoxHeightRatio->value() ) * h);
		if (w == 0) w = 1;
		ui.spinBoxWidth->blockSignals(true);
		ui.spinBoxWidth->setValue(w);
		ui.spinBoxWidth->blockSignals(false);
	}
	checkSelection();
}

void CustomSelectionDialog::checkSelection()
{
	QRect imageRect = QRect(QPoint(), imageSize);
	QRect selection = QRect(ui.spinBoxX->value(), ui.spinBoxY->value(), ui.spinBoxWidth->value(), ui.spinBoxHeight->value());
	if (imageRect.contains(selection))
	{
		ui.labelWarning->setText(QString());
	}
	else
	{
		ui.labelWarning->setText(QString(tr("The selection is too big!")));
	}
	ignoreButton->setVisible(!imageRect.contains(selection));
	ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(imageRect.contains(selection));
}

void CustomSelectionDialog::ignored(bool b)
{
	Q_UNUSED(b);
	ignoreButton->setVisible(false);
	ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void CustomSelectionDialog::selectARfromList(int i)
{
	if (i > 0)
	{
		switch(i)
		{
			case 1:
				ui.doubleSpinBoxWidthRatio->setValue(static_cast<double>(imageSize.width()) / imageSize.height());
				ui.doubleSpinBoxHeightRatio->setValue(1.0);
				break;
			case 2:
				ui.doubleSpinBoxWidthRatio->setValue(1.0);
				ui.doubleSpinBoxHeightRatio->setValue(1.0);
				break;
			case 3:
				ui.doubleSpinBoxWidthRatio->setValue(5.0);
				ui.doubleSpinBoxHeightRatio->setValue(4.0);
				break;
			case 4:
				ui.doubleSpinBoxWidthRatio->setValue(4.0);
				ui.doubleSpinBoxHeightRatio->setValue(3.0);
				break;
			case 5:
				ui.doubleSpinBoxWidthRatio->setValue(3.0);
				ui.doubleSpinBoxHeightRatio->setValue(2.0);
				break;
			case 6:
				ui.doubleSpinBoxWidthRatio->setValue(14.0);
				ui.doubleSpinBoxHeightRatio->setValue(9.0);
				break;
			case 7:
				ui.doubleSpinBoxWidthRatio->setValue(16.0);
				ui.doubleSpinBoxHeightRatio->setValue(10.0);
				break;
			case 8:
				ui.doubleSpinBoxWidthRatio->setValue(1.66);
				ui.doubleSpinBoxHeightRatio->setValue(1.0);
				break;
			case 9:
				ui.doubleSpinBoxWidthRatio->setValue(16.0);
				ui.doubleSpinBoxHeightRatio->setValue(9.0);
				break;
			case 10:
				ui.doubleSpinBoxWidthRatio->setValue(1.85);
				ui.doubleSpinBoxHeightRatio->setValue(1.0);
				break;
			case 11:
				ui.doubleSpinBoxWidthRatio->setValue(2.0);
				ui.doubleSpinBoxHeightRatio->setValue(1.0);
				break;
			case 12:
				ui.doubleSpinBoxWidthRatio->setValue(21.0);
				ui.doubleSpinBoxHeightRatio->setValue(9.0);
				break;
			case 13:
				ui.doubleSpinBoxWidthRatio->setValue(2.35);
				ui.doubleSpinBoxHeightRatio->setValue(1.0);
				break;
			case 14:
				ui.doubleSpinBoxWidthRatio->setValue(2.39);
				ui.doubleSpinBoxHeightRatio->setValue(1.0);
				break;
			case 15:
				ui.doubleSpinBoxWidthRatio->setValue(9.0);
				ui.doubleSpinBoxHeightRatio->setValue(16.0);
				break;
			case 16:
				ui.doubleSpinBoxWidthRatio->setValue(9.0);
				ui.doubleSpinBoxHeightRatio->setValue(18.0);
				break;
			case 17:
				ui.doubleSpinBoxWidthRatio->setValue(9.0);
				ui.doubleSpinBoxHeightRatio->setValue(19.5);
				break;
			case 18:
				ui.doubleSpinBoxWidthRatio->setValue(9.0);
				ui.doubleSpinBoxHeightRatio->setValue(20.0);
				break;
			default:
				break;
		}
		ui.comboBoxAR->setCurrentIndex(0);
	}
}

QRect CustomSelectionDialog::getSelection()
{
	QRect imageRect = QRect(QPoint(), imageSize);
	QRect selection = QRect(ui.spinBoxX->value(), ui.spinBoxY->value(), ui.spinBoxWidth->value(), ui.spinBoxHeight->value());
	return imageRect.intersected(selection);
}

void CustomSelectionDialog::savePreferences()
{
	if (inputSelection.isNull())
	{
		Globals::prefs->storeSpecificParameter("CustomSelectionDialog", "x", ui.spinBoxX->value());
		Globals::prefs->storeSpecificParameter("CustomSelectionDialog", "y", ui.spinBoxY->value());
		Globals::prefs->storeSpecificParameter("CustomSelectionDialog", "width", ui.spinBoxWidth->value());
		Globals::prefs->storeSpecificParameter("CustomSelectionDialog", "height", ui.spinBoxHeight->value());
		if (ui.checkBoxAR->isChecked())
		{
			Globals::prefs->storeSpecificParameter("CustomSelectionDialog", "widthRatio", ui.doubleSpinBoxWidthRatio->value());
			Globals::prefs->storeSpecificParameter("CustomSelectionDialog", "heightRatio", ui.doubleSpinBoxHeightRatio->value());
		}
		Globals::prefs->storeSpecificParameter("CustomSelectionDialog", "lockAR", ui.checkBoxAR->isChecked());
	}
}

