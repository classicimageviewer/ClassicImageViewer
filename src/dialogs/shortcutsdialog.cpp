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


#include "shortcutsdialog.h"
#include <QDebug>
#include <QStandardItemModel>
#include <stdio.h>
#include <stdlib.h>

ShortcutsDialog::ShortcutsDialog(QStringList shortCutInfo, QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);
	
	shortCutInfo.sort();
	
	QStandardItemModel * model = new QStandardItemModel(ui.tableView);
	ui.tableView->setModel(model);
	model->setHorizontalHeaderItem(0, new QStandardItem(tr("Shortcut")));
	model->setHorizontalHeaderItem(1, new QStandardItem(tr("Action")));
	
	for (int i = 0; i < shortCutInfo.size(); i++)
	{
		QStringList rowList = shortCutInfo.at(i).split('&');
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

ShortcutsDialog::~ShortcutsDialog()
{
	
}

