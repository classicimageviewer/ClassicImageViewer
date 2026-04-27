// Copyright (C) 2026 zhuvoy
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


#include "seamcarvingdialog.h"
#include "globals.h"
#include <QDebug>
#include "lib/imageOp.h"

SeamCarvingDialog::SeamCarvingDialog(QImage image, QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);
	setFixedSize(size());
	
	//apply preferences
	direction = Globals::prefs->fetchSpecificParameter("SeamCarvingDialog", "direction", 0).toInt();
	reduction = Globals::prefs->fetchSpecificParameter("SeamCarvingDialog", "reduction", 0).toInt();
	
	srcImg = ImageOp::Sharpen(image, 0.05).scaled(320*Globals::scalingFactor, 320*Globals::scalingFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.labelDst->setAlignment(Qt::AlignCenter);
	previewScale = (double)srcImg.width() / (double)image.width();
	
	ui.comboBoxDirection->setCurrentIndex(direction);
	ui.spinBoxReduction->setValue(reduction);
	ui.horizontalSliderReduction->setValue(reduction);
	
	connect(ui.comboBoxDirection, SIGNAL(activated(int)), this, SLOT(changeDirection(int)));
	connect(ui.spinBoxReduction, SIGNAL(valueChanged(int)), this, SLOT(spinBoxChanged(int)));
	connect(ui.horizontalSliderReduction, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));

	displayPreview();
}

SeamCarvingDialog::~SeamCarvingDialog()
{
	
}

void SeamCarvingDialog::spinBoxChanged(int value)
{
	reduction = value;
	ui.horizontalSliderReduction->blockSignals(true);
	ui.horizontalSliderReduction->setValue(reduction);
	ui.horizontalSliderReduction->blockSignals(false);
	displayPreview();
}

void SeamCarvingDialog::sliderChanged(int value)
{
	Q_UNUSED(value);
	reduction = ui.horizontalSliderReduction->value();
	ui.spinBoxReduction->blockSignals(true);
	ui.spinBoxReduction->setValue(reduction);
	ui.spinBoxReduction->blockSignals(false);
	displayPreview();
}

void SeamCarvingDialog::changeDirection(int v)
{
	direction = v;
	displayPreview();
}

void SeamCarvingDialog::displayPreview()
{
	QImage dst = shrinkImage(srcImg, previewScale).scaled(320*Globals::scalingFactor, 320*Globals::scalingFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QPixmap dstPix = QPixmap::fromImage(dst);
	dstPix.setDevicePixelRatio(Globals::scalingFactor);
	ui.labelDst->setPixmap(dstPix);
}

QImage SeamCarvingDialog::shrinkImage(QImage i, double scale)
{
	int reduction = this->reduction * scale;
	if (direction == 1)
	{
		return ImageOp::SeamCarvingVertical(i, reduction);
	}
	else
	{
		return ImageOp::SeamCarvingHorizontal(i, reduction);
	}
}

void SeamCarvingDialog::savePreferences()
{
	Globals::prefs->storeSpecificParameter("SeamCarvingDialog", "direction", direction);
	Globals::prefs->storeSpecificParameter("SeamCarvingDialog", "reduction", reduction);
}

