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


#ifndef MACROCONFIGDIALOG_H
#define MACROCONFIGDIALOG_H

#include <QDialog>
#include "ui_macroconfigdialog.h"
#include "ui_macroconfigresizedialog.h"
#include <QCheckBox>
#include <QComboBox>
#include <QTableView>
#include <QStandardItemModel>
#include "lib/macroEngine.h"


class MacroConfigResizeDialog : public QDialog
{
	Q_OBJECT
private:
	Ui_MacroConfigResizeDialog ui;
private slots:
	void resizeModeChanged(int index);
public:
	MacroConfigResizeDialog(QMap<QString, QVariant> intialConfig, QWidget * parent = NULL);
	~MacroConfigResizeDialog();
	QMap<QString, QVariant> getConfig();
};

/////__________
/////

class MacroConfigDialog : public QDialog
{
	Q_OBJECT
private:
	Ui_MacroConfigDialog ui;
	QCheckBox * enabled[9];
	QLineEdit * userNote[9];
	QComboBox * input[9];
	QTableView * macroList[9];
	QTableView * opList[9];
	QPushButton * deleteBtn[9];
	QPushButton * addBtn[9];
	QCheckBox * outputCopyToClipboard[9];
	QComboBox * outputImage[9];
	QList<QMap<QString, QVariant>> macro[9];
	QMap<int, QString> displayNameList;
	QImage inputImage;
	MacroEngine * macroEngine;
	
	bool eventFilter(QObject* watched, QEvent* event);
	void deleteBtnSlot(int page);
	void addBtnSlot(int page);
	void clearMacroSlot(int page);
	void populateOpList(QStandardItemModel * model, QString text, MacroEngine::Op op);
	void addOpSlot(int page, MacroEngine::Op op);
	void rebuildMacroList(int page);
	QMap<QString, QVariant> configureOp(MacroEngine::Op op, QList<QMap<QString, QVariant>> precedingMacro, QMap<QString, QVariant> initialConfig = QMap<QString, QVariant>());
	void reconfigureOp(int page, int row);
	QString translateOpToDisplayName(MacroEngine::Op op);
	QImage defaultImage();
public:
	MacroConfigDialog(QImage inputImage, QWidget * parent = NULL);
	~MacroConfigDialog();
	static QList<QMap<QString, QVariant>> getMacro(int index);
	static bool isMacroEnabled(int index);
	static int macroInputConfig(int index);
	static int macroOutputConfig(int index);
	static bool macroOutputClipboard(int index);
	void savePreferences();
};

#endif //MACROCONFIGDIALOG_H
