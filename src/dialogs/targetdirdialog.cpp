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


#include "targetdirdialog.h"
#include <QDebug>
#include <QFileDialog>

TargetDirDialog::TargetDirDialog(QString dir, QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);
	ui.lineEdit->setText(dir);
	connect(ui.pushButton, SIGNAL(clicked(bool)), this, SLOT(selectDir(bool)));
}

TargetDirDialog::~TargetDirDialog()
{
	
}

QString TargetDirDialog::getTargetDir()
{
	return ui.lineEdit->text();
}

void TargetDirDialog::selectDir(bool b)
{
	Q_UNUSED(b);
	QString newTarget = QFileDialog::getExistingDirectory(this, tr("Select Target Directory"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (!newTarget.isEmpty())
	{
		ui.lineEdit->setText(newTarget);
	}
}

