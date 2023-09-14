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


#include "aboutdialog.h"
#include <QDebug>
#include "globals.h"
#include "civ_version.h"

AboutDialog::AboutDialog(QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);
	QString versionText = QString(tr("Version: %1")).arg(CIV_VERSION);
	versionText += QString("\nQt %1").arg(qVersion());
	ui.labelVersion->setText(versionText);
	QImage img;
	if (Globals::scalingFactor >= 2)
	{
		img = QImage(":/pixmaps/pixmaps/logo@2x.png");
	}
	else
	{
		img = QImage(":/pixmaps/pixmaps/logo.png");
	}
	img.setDevicePixelRatio(Globals::scalingFactor);
	QPixmap pix = QPixmap::fromImage(img);
	pix.setDevicePixelRatio(Globals::scalingFactor);
	ui.labelLogo->setPixmap(pix);
	setFixedSize(size());
}

AboutDialog::~AboutDialog()
{
	
}

