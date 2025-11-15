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


#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QTableWidget>
#include "ui_preferencesdialog.h"
#include "io/imageIO.h"

class PreferencesDialog : public QDialog
{
	Q_OBJECT
private:
	Ui_PreferencesDialog ui;
	QPushButton * restoreDefaultButton;
	ImageIO * imageIO;
	QList<QCheckBox*> fileFormatSettings;
	QList<QLineEdit*> extraFileFormatSettings;
	QList<QCheckBox*> unlistedFileFormatSettings;
	QCheckBox * addFileFormatChechbox(QTableWidget * tableWidget, int row, int column, QString moduleName, QStringList list, QString listName, QString extension);
	void toggleFileFormatChechbox(QString prefix);
	void createFileFormatSettings();
	void saveFileFormatSettings();
private slots:
	void displayModeChanged(int i);
	void restoreDefaults(bool b);
	void changeFileFormatsTab(int i);
public:
	PreferencesDialog(ImageIO * imageIO, QWidget * parent = NULL);
	~PreferencesDialog();
	void savePreferences();

};

#endif //PREFERENCESDIALOG_H
