// Copyright (C) 2025 zhuvoy
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


#include "exttoolconfigdialog.h"
#include "globals.h"
#include <QDebug>
#include <QMessageBox>
#include <QPushButton>

ExtToolConfigDialog::ExtToolConfigDialog(QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);
	
	QString shell;
	int type;
	QString path;
	
  #define FETCH_TOOL_PARAMETERS(X) \
	shell = Globals::prefs->fetchSpecificParameter(QString("ExtToolConfig%1").arg(X), "shell", "").toString(); \
	if (shell.isEmpty()) \
	{ \
		shell = "/bin/sh"; \
	} \
	ui.lineEditShell ## X->setText(shell); \
	 \
	type = Globals::prefs->fetchSpecificParameter(QString("ExtToolConfig%1").arg(X), "type", 0).toInt(); \
	ui.comboBox ## X->setCurrentIndex(type); \
	 \
	path = Globals::prefs->fetchSpecificParameter(QString("ExtToolConfig%1").arg(X), "path", "").toString(); \
	ui.lineEditScript ## X->setText(path); \
	//
	
	FETCH_TOOL_PARAMETERS(1);
	FETCH_TOOL_PARAMETERS(2);
	FETCH_TOOL_PARAMETERS(3);
	FETCH_TOOL_PARAMETERS(4);
	FETCH_TOOL_PARAMETERS(5);
	FETCH_TOOL_PARAMETERS(6);
	FETCH_TOOL_PARAMETERS(7);
	FETCH_TOOL_PARAMETERS(8);
	FETCH_TOOL_PARAMETERS(9);
  #undef FETCH_TOOL_PARAMETERS
	
	connect(ui.buttonBox->button(QDialogButtonBox::Help), SIGNAL(clicked(bool)), this, SLOT(help(bool)));
	
  #define CONFIGURE_SELECT_BUTTONS(X) \
	ui.pushButtonSelect ## X->setIcon(this->style()->standardIcon(QStyle::SP_DialogOpenButton)); \
	ui.pushButtonSelect ## X->setToolTip(tr("Select file")); \
	QObject::connect(ui.pushButtonSelect ## X, &QPushButton::clicked, [=](bool checked) { \
		Q_UNUSED(checked); \
		QFileDialog dialog(this); \
		dialog.setFileMode(QFileDialog::AnyFile); \
		dialog.setViewMode(QFileDialog::Detail); \
		QString dirPath = Globals::prefs->fetchSpecificParameter(QString("ExtToolConfig"), "dirSelect", "").toString(); \
		if (!dirPath.isEmpty()) \
		{ \
			dialog.setDirectory(dirPath); \
		} \
		QStringList fileNames; \
		if (dialog.exec()) \
		{ \
			fileNames = dialog.selectedFiles(); \
			ui.lineEditScript ## X->setText(fileNames[0]); \
			Globals::prefs->storeSpecificParameter(QString("ExtToolConfig"), "dirSelect", dialog.directory().absolutePath() ); \
		} \
	}); \
	//
	
	CONFIGURE_SELECT_BUTTONS(1);
	CONFIGURE_SELECT_BUTTONS(2);
	CONFIGURE_SELECT_BUTTONS(3);
	CONFIGURE_SELECT_BUTTONS(4);
	CONFIGURE_SELECT_BUTTONS(5);
	CONFIGURE_SELECT_BUTTONS(6);
	CONFIGURE_SELECT_BUTTONS(7);
	CONFIGURE_SELECT_BUTTONS(8);
	CONFIGURE_SELECT_BUTTONS(9);
  #undef CONFIGURE_SELECT_BUTTONS
	
}

ExtToolConfigDialog::~ExtToolConfigDialog()
{
	
}

void ExtToolConfigDialog::help(bool b)
{
	Q_UNUSED(b);
	QString helpText;
	helpText += tr(	"External tools are scripts, that takes two argument:\n"
			"1) input file path\n"
			"2) output file path\n"
			"Both paths are provided to these scripts.\n"
			"Shell can be for example /bin/sh or /usr/bin/python.\n"
			"The input and output file type shall be the same.\n"
			"The output file shall be created via the script.\n"
			"Both files are deleted automatically.\n"
			"\n"
			"Example sh script (copy the image unaltered):\n"
		);
	helpText += "#!/bin/sh\n";
	helpText += "cp $1 $2\n";
	QMessageBox::information(this, tr("Help"), helpText);
}

void ExtToolConfigDialog::savePreferences()
{
	QString shell;
	int type;
	QString path;
	
  #define SAVE_TOOL_PARAMETERS(X) \
	shell = ui.lineEditShell ## X->text(); \
	Globals::prefs->storeSpecificParameter(QString("ExtToolConfig%1").arg(X), "shell", shell); \
	 \
	type = ui.comboBox ## X->currentIndex(); \
	Globals::prefs->storeSpecificParameter(QString("ExtToolConfig%1").arg(X), "type", type); \
	 \
	path = ui.lineEditScript ## X->text(); \
	Globals::prefs->storeSpecificParameter(QString("ExtToolConfig%1").arg(X), "path", path); \
	//
	
	SAVE_TOOL_PARAMETERS(1);
	SAVE_TOOL_PARAMETERS(2);
	SAVE_TOOL_PARAMETERS(3);
	SAVE_TOOL_PARAMETERS(4);
	SAVE_TOOL_PARAMETERS(5);
	SAVE_TOOL_PARAMETERS(6);
	SAVE_TOOL_PARAMETERS(7);
	SAVE_TOOL_PARAMETERS(8);
	SAVE_TOOL_PARAMETERS(9);
  #undef SAVE_TOOL_PARAMETERS
}

