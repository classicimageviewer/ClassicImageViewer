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


#include "resizedialog.h"
#include "globals.h"
#include <cmath>
#include <QDebug>

ResizeDialog::ResizeDialog(int width, int height, QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);
	setFixedSize(size());
	w = width;
	h = height;
	inputAR = (double)w / (double)h;
	ui.labelInfoCurrentResDisplay->setText(QString(tr("%1 x %2")).arg(w).arg(h));
	ui.spinBoxPixWidth->setValue(w);
	ui.spinBoxPixHeight->setValue(h);
	ui.doubleSpinBoxPercentWidth->setValue(100.0);
	ui.doubleSpinBoxPercentHeight->setValue(100.0);

	connect(ui.radioButtonNewResAsPixels, SIGNAL(clicked(bool)), this, SLOT(inputToPixelsChanged(bool)));
	connect(ui.radioButtonNewResAsPercent, SIGNAL(clicked(bool)), this, SLOT(inputToPercentsChanged(bool)));
	connect(ui.spinBoxPixWidth, SIGNAL(valueChanged(int)), this, SLOT(spinBoxPixWidthChanged(int)));
	connect(ui.spinBoxPixHeight, SIGNAL(valueChanged(int)), this, SLOT(spinBoxPixHeightChanged(int)));
	connect(ui.doubleSpinBoxPercentWidth, SIGNAL(valueChanged(double)), this, SLOT(spinBoxPercWidthChanged(double)));
	connect(ui.doubleSpinBoxPercentHeight, SIGNAL(valueChanged(double)), this, SLOT(spinBoxPercHeightChanged(double)));
	
	connect(ui.checkBoxAR, SIGNAL(stateChanged(int)), this, SLOT(checkBoxARchanged(int)));
	
	ui.comboBoxAlgorithm->addItems(Resizer::getListOfAlgorithms());
	
	//apply preferences
	QString input = Globals::prefs->fetchSpecificParameter("ResizeDialog", "input", QString("pixel")).toString();
	if (input == "percent")
	{
		ui.radioButtonNewResAsPercent->setChecked(true);
		inputToPercentsChanged(true);
	}
	else
	{
		ui.radioButtonNewResAsPixels->setChecked(true);
		inputToPixelsChanged(true);
	}
	ui.checkBoxAR->setChecked(Globals::prefs->fetchSpecificParameter("ResizeDialog", "lockAR", QVariant(true)).toBool());
	ui.comboBoxAlgorithm->setCurrentIndex(Globals::prefs->fetchSpecificParameter("ResizeDialog", "algo", QVariant(1)).toInt());
	
	displayOutputRes();
}

ResizeDialog::~ResizeDialog()
{
	
}

void ResizeDialog::inputToPixelsChanged(bool b)
{
	Q_UNUSED(b);
	ui.radioButtonNewResAsPercent->setChecked(false);
	ui.spinBoxPixWidth->setEnabled(true);
	ui.spinBoxPixHeight->setEnabled(true);
	ui.doubleSpinBoxPercentWidth->setEnabled(false);
	ui.doubleSpinBoxPercentHeight->setEnabled(false);
	ui.spinBoxPixWidth->setFocus(Qt::TabFocusReason);
	displayOutputRes();
}

void ResizeDialog::inputToPercentsChanged(bool b)
{
	Q_UNUSED(b);
	ui.radioButtonNewResAsPixels->setChecked(false);
	ui.spinBoxPixWidth->setEnabled(false);
	ui.spinBoxPixHeight->setEnabled(false);
	ui.doubleSpinBoxPercentWidth->setEnabled(true);
	ui.doubleSpinBoxPercentHeight->setEnabled(true);
	ui.doubleSpinBoxPercentWidth->setFocus(Qt::TabFocusReason);
	displayOutputRes();
}

void ResizeDialog::spinBoxPixWidthChanged(int value)
{
	if (ui.checkBoxAR->isChecked())
	{
		int n = std::round(value / inputAR);
		if (n < 1) n = 1;
		ui.spinBoxPixHeight->blockSignals(true);
		ui.spinBoxPixHeight->setValue(n);
		ui.spinBoxPixHeight->blockSignals(false);
	}
	displayOutputRes();
}

void ResizeDialog::spinBoxPixHeightChanged(int value)
{
	if (ui.checkBoxAR->isChecked())
	{
		int n = std::round(value * inputAR);
		if (n < 1) n = 1;
		ui.spinBoxPixWidth->blockSignals(true);
		ui.spinBoxPixWidth->setValue(n);
		ui.spinBoxPixWidth->blockSignals(false);
	}
	displayOutputRes();
}

void ResizeDialog::spinBoxPercWidthChanged(double value)
{
	if (ui.checkBoxAR->isChecked())
	{
		ui.doubleSpinBoxPercentHeight->blockSignals(true);
		ui.doubleSpinBoxPercentHeight->setValue(value);
		ui.doubleSpinBoxPercentHeight->blockSignals(false);
	}
	displayOutputRes();
}

void ResizeDialog::spinBoxPercHeightChanged(double value)
{
	if (ui.checkBoxAR->isChecked())
	{
		ui.doubleSpinBoxPercentWidth->blockSignals(true);
		ui.doubleSpinBoxPercentWidth->setValue(value);
		ui.doubleSpinBoxPercentWidth->blockSignals(false);
	}
	displayOutputRes();
}

void ResizeDialog::checkBoxARchanged(int i)
{
	Q_UNUSED(i);
	
	if (ui.checkBoxAR->isChecked())
	{
		spinBoxPixWidthChanged(ui.spinBoxPixWidth->value());
		spinBoxPercWidthChanged(ui.doubleSpinBoxPercentWidth->value());
	}
	displayOutputRes();
}

void ResizeDialog::displayOutputRes()
{
	if (ui.radioButtonNewResAsPixels->isChecked())
	{
		ow = ui.spinBoxPixWidth->value();
		oh = ui.spinBoxPixHeight->value();
	}
	else
	{
		ow = std::round(w * ui.doubleSpinBoxPercentWidth->value() * 0.01);
		oh = std::round(h * ui.doubleSpinBoxPercentHeight->value() * 0.01);
	}
	if (ow < 1) ow = 1;
	if (oh < 1) oh = 1;
	ui.labelInfoNewResDisplay->setText(QString(tr("%1 x %2")).arg(ow).arg(oh));
}

QSize ResizeDialog::getNewResolution()
{
	return QSize(ow, oh);
}

Resizer::Algorithm ResizeDialog::getAlgorithm()
{
	return (Resizer::Algorithm)(ui.comboBoxAlgorithm->currentIndex());
}

void ResizeDialog::savePreferences()
{
	Globals::prefs->storeSpecificParameter("ResizeDialog", "input", ui.radioButtonNewResAsPixels->isChecked() ? "pixel":"percent");
	Globals::prefs->storeSpecificParameter("ResizeDialog", "lockAR", ui.checkBoxAR->isChecked());
	Globals::prefs->storeSpecificParameter("ResizeDialog", "algo", ui.comboBoxAlgorithm->currentIndex());
}

