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
	#define GETPREF(X, DV)	valueOf ## X = Globals::prefs->fetchSpecificParameter("ColorAdjDialog", #X, DV).toDouble();	ui.doubleSpinBox ## X->setValue(valueOf ## X);	ui.horizontalSlider ## X->setValue(valueOf ## X*100);
	GETPREF(Brightness, 1.0);
	GETPREF(Contrast, 1.0);
	GETPREF(Gamma, 1.0);
	GETPREF(Saturation, 1.0);
	GETPREF(Hue, 0.0);
	GETPREF(Red, 0.0);
	GETPREF(Green, 0.0);
	GETPREF(Blue, 0.0);
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
	CONNECTC(Saturation);
	CONNECTC(Hue);
	CONNECTC(Red);
	CONNECTC(Green);
	CONNECTC(Blue);
	#undef CONNECTC

	displayAdjusted();
}

ColorAdjDialog::~ColorAdjDialog()
{
	
}

void ColorAdjDialog::spinBoxChanged(double value)
{
	Q_UNUSED(value);
	#define COPYVAL(X)	valueOf ## X = ui.doubleSpinBox ## X->value();	ui.horizontalSlider ## X->blockSignals(true);	ui.horizontalSlider ## X->setValue(valueOf ## X*100);	ui.horizontalSlider ## X->blockSignals(false);
	COPYVAL(Brightness);
	COPYVAL(Contrast);
	COPYVAL(Gamma);
	COPYVAL(Saturation);
	COPYVAL(Hue);
	COPYVAL(Red);
	COPYVAL(Green);
	COPYVAL(Blue);
	#undef COPYVAL
	displayAdjusted();
}

void ColorAdjDialog::sliderChanged(int value)
{
	Q_UNUSED(value);
	#define COPYVAL(X)	valueOf ## X = ui.horizontalSlider ## X->value()/100.0;	ui.doubleSpinBox ## X->blockSignals(true);	ui.doubleSpinBox ## X->setValue(valueOf ## X);	ui.doubleSpinBox ## X->blockSignals(false);
	COPYVAL(Brightness);
	COPYVAL(Contrast);
	COPYVAL(Gamma);
	COPYVAL(Saturation);
	COPYVAL(Hue);
	COPYVAL(Red);
	COPYVAL(Green);
	COPYVAL(Blue);
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
	return ColorAdjust::AdjustColor(i, valueOfBrightness, valueOfContrast, valueOfGamma, valueOfSaturation, valueOfHue, valueOfRed, valueOfGreen, valueOfBlue);
}

void ColorAdjDialog::savePreferences()
{
	if (ui.checkBoxSavePref->isChecked())
	{
		#define SETPREF(X)	Globals::prefs->storeSpecificParameter("ColorAdjDialog", #X, valueOf ## X);
		SETPREF(Brightness);
		SETPREF(Contrast);
		SETPREF(Gamma);
		SETPREF(Saturation);
		SETPREF(Hue);
		SETPREF(Red);
		SETPREF(Green);
		SETPREF(Blue);
		#undef SETPREF
	}
}

