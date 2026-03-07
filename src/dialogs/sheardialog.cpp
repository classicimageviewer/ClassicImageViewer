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


#include "sheardialog.h"
#include "globals.h"
#include <QDebug>
#include <QColorDialog>
#include "lib/imageOp.h"

ShearDialog::ShearDialog(QImage image, QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);
	setFixedSize(size());
	
	//apply preferences
	direction = Globals::prefs->fetchSpecificParameter("ShearDialog", "direction", 0).toInt();
	shear = Globals::prefs->fetchSpecificParameter("ShearDialog", "shear", 0).toInt();
	fillMethod = Globals::prefs->fetchSpecificParameter("ShearDialog", "fill", 0).toInt();
	blur = Globals::prefs->fetchSpecificParameter("ShearDialog", "blur", 0.0).toDouble();
	backgroundColor = Globals::prefs->fetchSpecificParameter("ShearDialog", "backgroundColor", QColor(Qt::black)).value<QColor>();
	
	srcImg = image.scaled(320*Globals::scalingFactor, 320*Globals::scalingFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.labelSrc->setAlignment(Qt::AlignCenter);
	ui.labelDst->setAlignment(Qt::AlignCenter);
	QPixmap srcPix = QPixmap::fromImage(srcImg);
	srcPix.setDevicePixelRatio(Globals::scalingFactor);
	ui.labelSrc->setPixmap(srcPix);
	previewScale = (double)srcImg.width() / (double)image.width();
	
	ui.comboBoxDirection->setCurrentIndex(direction);
	ui.comboBoxFill->setCurrentIndex(fillMethod);
	ui.spinBoxShear->setValue(shear);
	ui.horizontalSliderShear->setValue(shear);
	ui.doubleSpinBoxBlur->setValue(blur);
	ui.horizontalSliderBlur->setValue(blur*10.0);
	
	ui.doubleSpinBoxBlur->setVisible(fillMethod == 0);
	ui.horizontalSliderBlur->setVisible(fillMethod == 0);
	if (fillMethod == 0)
	{
		ui.labelFillParamSpacer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
	}
	else
	{
		ui.labelFillParamSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	}
	ui.pushButtonColor->setVisible(fillMethod == 1);
	
	connect(ui.comboBoxDirection, SIGNAL(activated(int)), this, SLOT(changeDirection(int)));
	connect(ui.spinBoxShear, SIGNAL(valueChanged(int)), this, SLOT(spinBoxChanged(int)));
	connect(ui.horizontalSliderShear, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
	connect(ui.doubleSpinBoxBlur, SIGNAL(valueChanged(double)), this, SLOT(spinBoxChanged(double)));
	connect(ui.horizontalSliderBlur, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
	connect(ui.comboBoxFill, SIGNAL(activated(int)), this, SLOT(changeFillMethod(int)));
	connect(ui.pushButtonColor, SIGNAL(clicked(bool)), this, SLOT(changeBackgroundColor(bool)));

	displaySheared();
}

ShearDialog::~ShearDialog()
{
	
}

void ShearDialog::spinBoxChanged(int value)
{
	shear = value;
	ui.horizontalSliderShear->blockSignals(true);
	ui.horizontalSliderShear->setValue(shear);
	ui.horizontalSliderShear->blockSignals(false);
	displaySheared();
}

void ShearDialog::spinBoxChanged(double value)
{
	blur = value;
	ui.horizontalSliderBlur->blockSignals(true);
	ui.horizontalSliderBlur->setValue(blur*10.0);
	ui.horizontalSliderBlur->blockSignals(false);
	displaySheared();
}

void ShearDialog::sliderChanged(int value)
{
	Q_UNUSED(value);
	shear = ui.horizontalSliderShear->value();
	blur = ui.horizontalSliderBlur->value() / 10.0;
	ui.spinBoxShear->blockSignals(true);
	ui.spinBoxShear->setValue(shear);
	ui.spinBoxShear->blockSignals(false);
	ui.doubleSpinBoxBlur->blockSignals(true);
	ui.doubleSpinBoxBlur->setValue(blur);
	ui.doubleSpinBoxBlur->blockSignals(false);
	displaySheared();
}

void ShearDialog::changeDirection(int v)
{
	direction = v;
	displaySheared();
}

void ShearDialog::changeFillMethod(int v)
{
	fillMethod = v;
	ui.doubleSpinBoxBlur->setVisible(fillMethod == 0);
	ui.horizontalSliderBlur->setVisible(fillMethod == 0);
	if (fillMethod == 0)
	{
		ui.labelFillParamSpacer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
	}
	else
	{
		ui.labelFillParamSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	}

	ui.pushButtonColor->setVisible(fillMethod == 1);
	displaySheared();
}

void ShearDialog::changeBackgroundColor(bool b)
{
	Q_UNUSED(b);
	QColorDialog * d = new QColorDialog(backgroundColor);
	if (d->exec() == QDialog::Accepted)
	{
		backgroundColor = d->selectedColor();
	}
	delete d;
	displaySheared();
}

void ShearDialog::displaySheared()
{
	QImage dst = shearImage(srcImg, previewScale).scaled(320*Globals::scalingFactor, 320*Globals::scalingFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QPixmap dstPix = QPixmap::fromImage(dst);
	dstPix.setDevicePixelRatio(Globals::scalingFactor);
	ui.labelDst->setPixmap(dstPix);
}

QImage ShearDialog::shearImage(QImage i, double scale)
{
	QImage::Format tmpFormat = (i.hasAlphaChannel() || (fillMethod == 2)) ? QImage::Format_ARGB32 : QImage::Format_RGB32;
	double shearAmount = shear;
	shearAmount *= scale;
	QTransform transform;
	if (direction == 0)
	{
		shearAmount /= i.height();
		transform = QTransform().shear(shearAmount, 0);
	}
	else
	{
		shearAmount /= i.width();
		transform = QTransform().shear(0, shearAmount);
	}
	QSize sheardSize = transform.mapRect(i.rect()).size();
	QImage dst(sheardSize, tmpFormat);
	if (fillMethod == 0)
	{
		QTransform transformBack;
		if (direction == 0)
		{
			transformBack = QTransform().shear(-shearAmount, 0);
		}
		else
		{
			transformBack = QTransform().shear(0, -shearAmount);
		}
		QSize extendedSize = transformBack.mapRect(transform.mapRect(i.rect())).size();
		QImage extended(extendedSize, tmpFormat);
		extended.fill(QColorConstants::Transparent);
		int hx = (extendedSize.width() - i.width()) / 2;
		int hy = (extendedSize.height() - i.height())/2;
		
		if (i.hasAlphaChannel())
		{
			extended.fill(Qt::transparent);
		}
		else
		{
			extended.fill(Qt::black);
		}
		
		QPainter q(&extended);
		q.drawImage(QRect(0, hy, hx, i.height()), i, QRect(0, 0, 1, i.height()));					// left
		q.drawImage(QRect(hx + i.width(), hy, hx, i.height()), i, QRect(i.width() - 1, 0, 1, i.height()));		// right
		q.drawImage(QRect(hx, 0, i.width(), hy), i, QRect(0, 0, i.width(), 1));						// top
		q.drawImage(QRect(hx, hy + i.height(), i.width(), hy), i, QRect(0, i.height() - 1, i.width(), 1));		// bottom
		q.drawImage(QRect(0, 0, hx, hy), i, QRect(0, 0, 1, 1));								// top left
		q.drawImage(QRect(0, hy + i.height(), hx, hy), i, QRect(0, i.height() - 1, 1, 1));				// bottom left
		q.drawImage(QRect(hx + i.width(), 0, hx, hy), i, QRect(i.width() - 1, 0, 1, 1));				// top right
		q.drawImage(QRect(hx + i.width(), hy + i.height(), hx, hy), i, QRect(i.width() - 1, i.height() - 1, 1, 1));	// bottom right
		q.drawImage(hx, hy, i);
		q.end();
		
		if (blur > 0.0)
		{
			extended = ImageOp::Blur(extended, blur);
			
			QPainter q(&extended);
			q.drawImage(hx, hy, i);
			q.end();
		}
		
		QImage sheard = extended.transformed(transform, Qt::SmoothTransformation);
		if (i.hasAlphaChannel())
		{
			dst.fill(Qt::transparent);
		}
		else
		{
			dst.fill(backgroundColor);
		}
		QPainter p(&dst);
		p.drawImage((sheardSize.width() - sheard.width())/2, (sheardSize.height() - sheard.height())/2, sheard);
		p.end();
	}
	else
	{
		QImage sheard = i.transformed(transform, Qt::SmoothTransformation);
		if (fillMethod == 1)
		{
			dst.fill(backgroundColor);
		}
		else
		{
			dst.fill(Qt::transparent);
		}
		QPainter p(&dst);
		p.drawImage(0, 0, sheard);
		p.end();
	}
	dst = dst.convertToFormat(i.format());
	return dst;
}

void ShearDialog::savePreferences()
{
	Globals::prefs->storeSpecificParameter("ShearDialog", "direction", direction);
	Globals::prefs->storeSpecificParameter("ShearDialog", "shear", shear);
	Globals::prefs->storeSpecificParameter("ShearDialog", "fill", fillMethod);
	Globals::prefs->storeSpecificParameter("ShearDialog", "blur", blur);
	Globals::prefs->storeSpecificParameter("ShearDialog", "backgroundColor", backgroundColor);
}

