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


#include "coloradjdialog.h"
#include "globals.h"
#include "modules/coloradj.h"
#include <QDebug>
#include <QColorDialog>

ColorAdjDialog::ColorAdjDialog(QImage image, QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);
	setFixedSize(size());
	
	//apply preferences
	#define GETPREF(X, DV, SC)	valueOf ## X = Globals::prefs->fetchSpecificParameter("ColorAdjDialog", #X, DV).toDouble();	ui.doubleSpinBox ## X->setValue(valueOf ## X);	ui.horizontalSlider ## X->setValue(valueOf ## X*SC);
	GETPREF(Brightness, 0, 1);
	GETPREF(Contrast, 0, 1);
	GETPREF(Gamma, 1.0, 100.0);
	GETPREF(Exposure, 0, 100.0);
	GETPREF(Saturation, 0, 1);
	GETPREF(Hue, 0, 1);
	GETPREF(Red, 0, 1);
	GETPREF(Green, 0, 1);
	GETPREF(Blue, 0, 1);
	#undef GETPREF
	
	srcImg = image.scaled(320*Globals::scalingFactor, 320*Globals::scalingFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.labelSrc->setAlignment(Qt::AlignCenter);
	ui.labelDst->setAlignment(Qt::AlignCenter);
	QPixmap srcPix = QPixmap::fromImage(srcImg);
	srcPix.setDevicePixelRatio(Globals::scalingFactor);
	ui.labelSrc->setPixmap(srcPix);
	
	#define CONNECTC(X)	connect(ui.doubleSpinBox ## X, SIGNAL(valueChanged(double)), this, SLOT(spinBoxChanged(double)));	connect(ui.horizontalSlider ## X, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
	CONNECTC(Brightness);
	CONNECTC(Contrast);
	CONNECTC(Gamma);
	CONNECTC(Exposure);
	CONNECTC(Saturation);
	CONNECTC(Hue);
	CONNECTC(Red);
	CONNECTC(Green);
	CONNECTC(Blue);
	#undef CONNECTC
	
	restoreDefaultButton = ui.buttonBox->button(QDialogButtonBox::RestoreDefaults);
	connect(restoreDefaultButton, SIGNAL(clicked(bool)), this, SLOT(restoreDefaults(bool)));

	displayAdjusted();
}

ColorAdjDialog::~ColorAdjDialog()
{
	
}

void ColorAdjDialog::restoreDefaults(bool b)
{
	Q_UNUSED(b);
	#define SETVAL(X, DV, SC)	valueOf ## X = DV;	ui.doubleSpinBox ## X->blockSignals(true);	ui.horizontalSlider ## X->blockSignals(true);	ui.doubleSpinBox ## X->setValue(valueOf ## X);	ui.horizontalSlider ## X->setValue(valueOf ## X*SC);	ui.horizontalSlider ## X->blockSignals(false);	ui.doubleSpinBox ## X->blockSignals(false);
	SETVAL(Brightness, 0, 1);
	SETVAL(Contrast, 0, 1);
	SETVAL(Gamma, 1.0, 100.0);
	SETVAL(Exposure, 0.0, 100.0);
	SETVAL(Saturation, 0, 1);
	SETVAL(Hue, 0, 1);
	SETVAL(Red, 0, 1);
	SETVAL(Green, 0, 1);
	SETVAL(Blue, 0, 1);
	#undef SETVAL
	displayAdjusted();
}

void ColorAdjDialog::spinBoxChanged(double value)
{
	Q_UNUSED(value);
	#define COPYVAL(X, SC)	valueOf ## X = ui.doubleSpinBox ## X->value();	ui.horizontalSlider ## X->blockSignals(true);	ui.horizontalSlider ## X->setValue(valueOf ## X*SC);	ui.horizontalSlider ## X->blockSignals(false);
	COPYVAL(Brightness, 1);
	COPYVAL(Contrast, 1);
	COPYVAL(Gamma, 100);
	COPYVAL(Exposure, 100);
	COPYVAL(Saturation, 1);
	COPYVAL(Hue, 1);
	COPYVAL(Red, 1);
	COPYVAL(Green, 1);
	COPYVAL(Blue, 1);
	#undef COPYVAL
	displayAdjusted();
}

void ColorAdjDialog::sliderChanged(int value)
{
	Q_UNUSED(value);
	#define COPYVAL(X, SC)	valueOf ## X = ui.horizontalSlider ## X->value();	valueOf ## X /= SC;	ui.doubleSpinBox ## X->blockSignals(true);	ui.doubleSpinBox ## X->setValue(valueOf ## X);	ui.doubleSpinBox ## X->blockSignals(false);
	COPYVAL(Brightness, 1);
	COPYVAL(Contrast, 1);
	COPYVAL(Gamma, 100);
	COPYVAL(Exposure, 100);
	COPYVAL(Saturation, 1);
	COPYVAL(Hue, 1);
	COPYVAL(Red, 1);
	COPYVAL(Green, 1);
	COPYVAL(Blue, 1);
	#undef COPYVAL
	displayAdjusted();
}

void ColorAdjDialog::displayAdjusted()
{
	QImage dst = adjustColor(srcImg);
	QPixmap dstPix = QPixmap::fromImage(dst);
	dstPix.setDevicePixelRatio(Globals::scalingFactor);
	ui.labelDst->setPixmap(dstPix);
}

QImage ColorAdjDialog::adjustColor(QImage i)
{
	return ColorAdjust::AdjustColor(i, valueOfBrightness/100.0, valueOfContrast/100.0, valueOfGamma, valueOfExposure, valueOfSaturation/100.0, valueOfHue/180.0, valueOfRed/100.0, valueOfGreen/100.0, valueOfBlue/100.0);
}

void ColorAdjDialog::savePreferences()
{
	if (ui.checkBoxSavePref->isChecked())
	{
		#define SETPREF(X)	Globals::prefs->storeSpecificParameter("ColorAdjDialog", #X, valueOf ## X);
		SETPREF(Brightness);
		SETPREF(Contrast);
		SETPREF(Gamma);
		SETPREF(Exposure);
		SETPREF(Saturation);
		SETPREF(Hue);
		SETPREF(Red);
		SETPREF(Green);
		SETPREF(Blue);
		#undef SETPREF
	}
}

