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


#include "infodialog.h"
#include <QDebug>
#include <QProcess>
#include <QStandardItemModel>

InfoDialog::InfoDialog(QString fileAbsolutePath, QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);
	
	QStringList arguments;
	arguments.append("-t");
	arguments.append(fileAbsolutePath);
	
	QProcess exiftoolProc;
	exiftoolProc.start("exiftool", arguments);
	if (!exiftoolProc.waitForFinished(1000))
	{
		QStandardItemModel * model = new QStandardItemModel(ui.tableView);
		ui.tableView->setModel(model);
		model->setHorizontalHeaderItem(0, new QStandardItem(tr("This functionality requires 'exiftool' to be installed")));
		ui.tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
		ui.tableView->verticalHeader()->setVisible(false);
		return;
	}

	QString exiftoolOutput = QString(exiftoolProc.readAllStandardOutput());

	QStandardItemModel * model = new QStandardItemModel(ui.tableView);
	ui.tableView->setModel(model);
	model->setHorizontalHeaderItem(0, new QStandardItem(tr("Tag")));
	model->setHorizontalHeaderItem(1, new QStandardItem(tr("Value")));
	QStringList Lines = exiftoolOutput.split('\n');
	
	for (int i = 0; i < Lines.size(); i++)
	{
		QStringList rowList = Lines.at(i).split('\t');
		QList<QStandardItem*> rowData;
		if (rowList.size() == 2)
		{
			rowData.clear();
			rowData.append(new QStandardItem(rowList.at(0)));
			rowData.append(new QStandardItem(rowList.at(1)));
			model->appendRow(rowData);
		}
	}
	ui.tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	ui.tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	ui.tableView->verticalHeader()->setVisible(false);


}

InfoDialog::~InfoDialog()
{
	
}

