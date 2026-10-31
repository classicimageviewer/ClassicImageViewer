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


#ifndef SHORTCUTSDIALOG_H
#define SHORTCUTSDIALOG_H

#include <QDialog>
#include "ui_shortcutsdialog.h"

class ShortcutsDialog : public QDialog
{
	Q_OBJECT
private:
	Ui_ShortcutsDialog ui;
	QStringList shortCutInfo;
private slots:
	void onSearchTextChanged(const QString &text);
protected:
	void showEvent(QShowEvent *event) override;
	bool eventFilter(QObject *watched, QEvent *event) override;
private:
	void populateModel(const QString &filter = QString());
	bool matchesFilter(const QString &shortcut, const QString &action, const QString &filter);
public:
	ShortcutsDialog(QStringList shortCutInfo, QWidget * parent = NULL);
	~ShortcutsDialog();

};

#endif //SHORTCUTSDIALOG_H
