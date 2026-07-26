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


#include "newimagedialog.h"
#include "globals.h"
#include <QDebug>
#include <QColorDialog>

NewImageDialog::NewImageDialog(QMap<QString, QVariant> intialConfig, QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);
	setFixedSize(size());
	
	color = Globals::prefs->fetchSpecificParameter("NewImageDialog", "color", QColor(Qt::black)).value<QColor>();
	ui.spinBoxWidth->setValue(Globals::prefs->fetchSpecificParameter("NewImageDialog", "width", 640).toInt());
	ui.spinBoxHeight->setValue(Globals::prefs->fetchSpecificParameter("NewImageDialog", "height", 480).toInt());
	ui.comboBoxBPP->setCurrentIndex(Globals::prefs->fetchSpecificParameter("NewImageDialog", "bpp", 1).toInt());
	
	if (intialConfig.contains("color"))
	{
		color = intialConfig["color"].value<QColor>();
	}
	if (intialConfig.contains("width"))
	{
		ui.spinBoxWidth->setValue(intialConfig["width"].toInt());
	}
	if (intialConfig.contains("height"))
	{
		ui.spinBoxHeight->setValue(intialConfig["height"].toInt());
	}
	if (intialConfig.contains("bpp"))
	{
		ui.comboBoxBPP->setCurrentIndex(intialConfig["bpp"].toInt());
	}
	
	connect(ui.pushButtonColor, SIGNAL(clicked(bool)), this, SLOT(changeColor(bool)));
}

NewImageDialog::~NewImageDialog()
{
	
}

void NewImageDialog::changeColor(bool b)
{
	Q_UNUSED(b);
	QColorDialog * d = new QColorDialog(color);
	d->setOption(QColorDialog::ShowAlphaChannel);
	if (d->exec() == QDialog::Accepted)
	{
		color = d->selectedColor();
	}
	delete d;
}

QImage NewImageDialog::newImage()
{
	QImage::Format format;
	QColor fillColor;
	switch(ui.comboBoxBPP->currentIndex())
	{
		case 0:
			format = QImage::Format_Mono;
			fillColor = (color.value() >= 128) ? Qt::color1 : Qt::color0;
			break;
		default:
		case 1:
			format = QImage::Format_RGB32;
			fillColor = color;
			fillColor.setAlpha(255);
			break;
		case 2:
			format = QImage::Format_ARGB32;
			fillColor = color;
			break;
	}
	QImage image = QImage(ui.spinBoxWidth->value(), ui.spinBoxHeight->value(), format);
	image.fill(fillColor);
	return image;
}

void NewImageDialog::savePreferences()
{
	Globals::prefs->storeSpecificParameter("NewImageDialog", "color", color);
	Globals::prefs->storeSpecificParameter("NewImageDialog", "width", ui.spinBoxWidth->value());
	Globals::prefs->storeSpecificParameter("NewImageDialog", "height", ui.spinBoxHeight->value());
	Globals::prefs->storeSpecificParameter("NewImageDialog", "bpp", ui.comboBoxBPP->currentIndex());
}

QMap<QString, QVariant> NewImageDialog::getConfig()
{
	QMap<QString, QVariant> config;
	config["color"] = color;
	config["width"] = ui.spinBoxWidth->value();
	config["height"] = ui.spinBoxHeight->value();
	config["bpp"] = ui.comboBoxBPP->currentIndex();
	return config;
}

