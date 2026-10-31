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
#include <QHeaderView>
#include <QKeyEvent>
#include <QLineEdit>
#include <QShowEvent>
#include <stdio.h>
#include <stdlib.h>

ShortcutsDialog::ShortcutsDialog(QStringList shortCutInfo, QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);
	
	this->shortCutInfo = shortCutInfo;
	this->shortCutInfo.sort();
	
	QStandardItemModel * model = new QStandardItemModel(ui.tableView);
	ui.tableView->setModel(model);
	model->setHorizontalHeaderItem(0, new QStandardItem(tr("Shortcut")));
	model->setHorizontalHeaderItem(1, new QStandardItem(tr("Action")));
	
	populateModel();
	
	ui.tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	ui.tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	ui.tableView->verticalHeader()->setVisible(false);

	ui.lineEdit->setPlaceholderText(tr("Search shortcuts or actions..."));
	ui.lineEdit->setClearButtonEnabled(true);
	connect(ui.lineEdit, &QLineEdit::textChanged, this, &ShortcutsDialog::onSearchTextChanged);

	ui.lineEdit->installEventFilter(this);
	ui.lineEdit->setFocus();
}

ShortcutsDialog::~ShortcutsDialog()
{
	
}

void ShortcutsDialog::showEvent(QShowEvent *event)
{
	QDialog::showEvent(event);
	ui.lineEdit->setFocus();
}

bool ShortcutsDialog::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == ui.lineEdit && event->type() == QEvent::KeyPress)
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		if (keyEvent->key() == Qt::Key_Escape)
		{
			reject();
			return true;
		}
	}
	return QDialog::eventFilter(watched, event);
}

void ShortcutsDialog::onSearchTextChanged(const QString &text)
{
	const QString filter = text.trimmed();
	populateModel(filter);
}

void ShortcutsDialog::populateModel(const QString &filter)
{
	QStandardItemModel *model = qobject_cast<QStandardItemModel *>(ui.tableView->model());
	if (!model)
	{
		return;
	}

	model->removeRows(0, model->rowCount());

	for (const QString &entry : shortCutInfo)
	{
		QStringList rowList = entry.split('&');

		if (rowList.size() != 2) continue;

		const QString shortcut = rowList.at(0);
		const QString action = rowList.at(1);

		if (!matchesFilter(shortcut, action, filter)) continue;

		QList<QStandardItem *> rowData;
		rowData.append(new QStandardItem(shortcut));
		rowData.append(new QStandardItem(action));

		model->appendRow(rowData);
	}
}

bool ShortcutsDialog::matchesFilter(const QString &shortcut, const QString &action, const QString &filter)
{
	if (filter.isEmpty())
	{
		return true;
	}

	return shortcut.contains(filter, Qt::CaseInsensitive) || action.contains(filter, Qt::CaseInsensitive);
}

