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


#include "rotatedialog.h"
#include "globals.h"
#include <QDebug>
#include <QColorDialog>

RotateDialog::RotateDialog(QImage image, QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);
	setFixedSize(size());
	
	//apply preferences
	angle = Globals::prefs->fetchSpecificParameter("RotateDialog", "angle", 0.0).toDouble();
	backgroundColor = Globals::prefs->fetchSpecificParameter("RotateDialog", "backgroundColor", QColor(Qt::black)).value<QColor>();
	
	srcImg = image.scaled(320*Globals::scalingFactor, 320*Globals::scalingFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.labelSrc->setAlignment(Qt::AlignCenter);
	ui.labelDst->setAlignment(Qt::AlignCenter);
	QPixmap srcPix = QPixmap::fromImage(srcImg);
	srcPix.setDevicePixelRatio(Globals::scalingFactor);
	ui.labelSrc->setPixmap(srcPix);
	
	ui.doubleSpinBoxAngle->setValue(angle);
	ui.horizontalSliderAngle->setValue(angle*100.0);
	
	connect(ui.doubleSpinBoxAngle, SIGNAL(valueChanged(double)), this, SLOT(spinBoxChanged(double)));
	connect(ui.horizontalSliderAngle, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
	connect(ui.pushButtonColor, SIGNAL(clicked(bool)), this, SLOT(changeBackgroundColor(bool)));

	displayRotated();
}

RotateDialog::~RotateDialog()
{
	
}

void RotateDialog::spinBoxChanged(double value)
{
	angle = value;
	ui.horizontalSliderAngle->blockSignals(true);
	ui.horizontalSliderAngle->setValue(value*100.0);
	ui.horizontalSliderAngle->blockSignals(false);
	displayRotated();
}

void RotateDialog::sliderChanged(int value)
{
	angle = value / 100.0;
	ui.doubleSpinBoxAngle->blockSignals(true);
	ui.doubleSpinBoxAngle->setValue(angle);
	ui.doubleSpinBoxAngle->blockSignals(false);
	displayRotated();
}

void RotateDialog::changeBackgroundColor(bool b)
{
	Q_UNUSED(b);
	QColorDialog * d = new QColorDialog(backgroundColor);
	if (d->exec() == QDialog::Accepted)
	{
		backgroundColor = d->selectedColor();
	}
	delete d;
	displayRotated();
}

void RotateDialog::displayRotated()
{
	QImage dst = rotateImage(srcImg).scaled(320*Globals::scalingFactor, 320*Globals::scalingFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QPixmap dstPix = QPixmap::fromImage(dst);
	dstPix.setDevicePixelRatio(Globals::scalingFactor);
	ui.labelDst->setPixmap(dstPix);
}

QImage RotateDialog::rotateImage(QImage i)
{
	QImage rotated = i.transformed(QTransform().rotate(angle), Qt::SmoothTransformation);
	QImage dst(rotated.size(), QImage::Format_ARGB32);
	dst.fill(backgroundColor);
	QPainter p(&dst);
	p.drawImage(0, 0, rotated);
	p.end();
	dst = dst.convertToFormat(i.format());
	return dst;
}

void RotateDialog::savePreferences()
{
	Globals::prefs->storeSpecificParameter("RotateDialog", "angle", angle);
	Globals::prefs->storeSpecificParameter("RotateDialog", "backgroundColor", backgroundColor);
}

