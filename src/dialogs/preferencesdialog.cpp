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


#include "preferencesdialog.h"
#include "globals.h"
#include <QDebug>
#include <QMessageBox>
#include <QHeaderView>

PreferencesDialog::PreferencesDialog(ImageIO * imageIO, QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.comboBoxDisplayMode, SIGNAL(currentIndexChanged(int)), this, SLOT(displayModeChanged(int)));

	ui.comboBoxLocale->setCurrentIndex(0);
	QString userLocale = Globals::prefs->getUserLocale();
	ui.comboBoxLocale->addItem("English");
	ui.comboBoxLocale->addItem("Deutsch");
	if (userLocale == "en") ui.comboBoxLocale->setCurrentIndex(1);
	if (userLocale == "de") ui.comboBoxLocale->setCurrentIndex(2);
	
	ui.checkBoxStartMaximized->setChecked(Globals::prefs->getMaximizedWindow());
	ui.checkBoxStartFullscreen->setChecked(Globals::prefs->getStartFullscreen());
	ui.comboBoxFileOrder->setCurrentIndex(Globals::prefs->getFileOrder());
	ui.checkBoxLoopDir->setChecked(Globals::prefs->getLoopDir());
	ui.comboBoxDisplayMode->setCurrentIndex(Globals::prefs->getDisplayMode());
	ui.checkBoxFitWindowWhenZoomed->setChecked(Globals::prefs->getFitWindowWhenZoomed());
	ui.spinBoxDisplayBackground->setValue(Globals::prefs->getDisplayBackground());
	ui.spinBoxZoomDelta->setValue(Globals::prefs->getZoomDelta());
	ui.checkBoxReverseMouseWheel->setChecked(Globals::prefs->getReverseWheel());
	ui.checkBoxClearClipboard->setChecked(Globals::prefs->getClearClipboardOnExit());
	ui.checkBoxClearRecentFiles->setChecked(Globals::prefs->getClearRecentFilesOnExit());
	ui.comboBoxSelector->setCurrentIndex(Globals::prefs->getUseFastSelector() ? 1:0);
	ui.checkBoxEnableToolbarShrinking->setChecked(Globals::prefs->getEnableToolbarShrinking());
	ui.comboBoxDisplayQuality->setCurrentIndex(Globals::prefs->getDisplayHigherQuality() ? 1:0);
	
	ui.checkBoxFsHideCursor->setChecked(Globals::prefs->getFullscreenHideCursor());
	ui.comboBoxFsDisplayMode->setCurrentIndex(Globals::prefs->getFullscreenDisplayMode());
	ui.checkBoxFsFileIndexIndicator->setChecked(Globals::prefs->getFullscreenFileIndexIndicator());
	ui.checkBoxFsFileNameIndicator->setChecked(Globals::prefs->getFullscreenFileNameIndicator());
	QString indicatorColor = Globals::prefs->getFullscreenIndicatorColor();
	ui.comboBoxFsIndicatorColor->setCurrentIndex(7);
	if (indicatorColor == "darkGray") ui.comboBoxFsIndicatorColor->setCurrentIndex(0);
	if (indicatorColor == "gray") ui.comboBoxFsIndicatorColor->setCurrentIndex(1);
	if (indicatorColor == "lightGray") ui.comboBoxFsIndicatorColor->setCurrentIndex(2);
	if (indicatorColor == "white") ui.comboBoxFsIndicatorColor->setCurrentIndex(3);
	if (indicatorColor == "darkRed") ui.comboBoxFsIndicatorColor->setCurrentIndex(4);
	if (indicatorColor == "red") ui.comboBoxFsIndicatorColor->setCurrentIndex(5);
	if (indicatorColor == "darkGreen") ui.comboBoxFsIndicatorColor->setCurrentIndex(6);
	if (indicatorColor == "green") ui.comboBoxFsIndicatorColor->setCurrentIndex(7);
	if (indicatorColor == "darkBlue") ui.comboBoxFsIndicatorColor->setCurrentIndex(8);
	if (indicatorColor == "blue") ui.comboBoxFsIndicatorColor->setCurrentIndex(9);
	if (!Globals::prefs->getFullscreenIndicatorFontFamily().isEmpty())
	{
		ui.fontComboBoxFsIndicatorFontFamily->setCurrentFont(QFont(Globals::prefs->getFullscreenIndicatorFontFamily()));
	}
	ui.spinBoxFsIndicatorFontSize->setValue(Globals::prefs->getFullscreenIndicatorFontSize());
	
	ui.spinBoxUsMinimumSteps->setValue(Globals::prefs->getUndoStackMinimumSteps());
	ui.spinBoxUsMemoryLimit->setValue(Globals::prefs->getUndoStackMemoryLimit());
	
	ui.spinBoxThCacheSize->setValue(Globals::prefs->getThumbnailsCacheSize());
	ui.checkBoxThPreloading->setChecked(Globals::prefs->getThumbnailsPreloading());
	ui.comboBoxThMultithreading->setCurrentIndex(Globals::prefs->getThumbnailsThreads());
	ui.spinBoxThScrollSpeed->setValue(Globals::prefs->getThumbnailsScrollSpeed());
	
	QStringList extEditorList = Globals::prefs->getExternalEditors();
	QString extEditor1, extEditor2, extEditor3, extEditor4;
	if (extEditorList.length() >= 1) extEditor1 = extEditorList.at(0);
	if (extEditorList.length() >= 2) extEditor2 = extEditorList.at(1);
	if (extEditorList.length() >= 3) extEditor3 = extEditorList.at(2);
	if (extEditorList.length() >= 4) extEditor4 = extEditorList.at(3);
	extEditorList = extEditor1.split(";");
	ui.lineEditExtEditor1Cmd->setText(extEditorList.at(0));
	if (extEditorList.length() > 1)
	{
		extEditorList.removeAt(0);
		ui.lineEditExtEditor1Args->setText(extEditorList.join(";"));
	}
	extEditorList = extEditor2.split(";");
	ui.lineEditExtEditor2Cmd->setText(extEditorList.at(0));
	if (extEditorList.length() > 1)
	{
		extEditorList.removeAt(0);
		ui.lineEditExtEditor2Args->setText(extEditorList.join(";"));
	}
	extEditorList = extEditor3.split(";");
	ui.lineEditExtEditor3Cmd->setText(extEditorList.at(0));
	if (extEditorList.length() > 1)
	{
		extEditorList.removeAt(0);
		ui.lineEditExtEditor3Args->setText(extEditorList.join(";"));
	}
	extEditorList = extEditor4.split(";");
	ui.lineEditExtEditor4Cmd->setText(extEditorList.at(0));
	if (extEditorList.length() > 1)
	{
		extEditorList.removeAt(0);
		ui.lineEditExtEditor4Args->setText(extEditorList.join(";"));
	}

	this->imageIO = imageIO;
	createFileFormatSettings();
	
	restoreDefaultButton = ui.buttonBox->button(QDialogButtonBox::RestoreDefaults);
	connect(restoreDefaultButton, SIGNAL(clicked(bool)), this, SLOT(restoreDefaults(bool)));
}

PreferencesDialog::~PreferencesDialog()
{
	
}

void PreferencesDialog::restoreDefaults(bool b)
{
	Q_UNUSED(b);
	if (QMessageBox::Yes == QMessageBox::question(this, tr("Restore defaults"), tr("Do you really want to clear and restore all settings to the default?\nThese includes previous files, effect parameter and more...\nThe application will exit.")))
	{
		Globals::prefs->restoreDefaults();
		qApp->quit();
	}
}

void PreferencesDialog::displayModeChanged(int i)
{
	Q_UNUSED(i);
	ui.checkBoxFitWindowWhenZoomed->setEnabled(ui.comboBoxDisplayMode->currentIndex() == 0);
}

void PreferencesDialog::savePreferences()
{
	switch (ui.comboBoxLocale->currentIndex())
	{
		default:
		case 0:
			Globals::prefs->setUserLocale("");
			break;
		case 1:
			Globals::prefs->setUserLocale("en");
			break;
		case 2:
			Globals::prefs->setUserLocale("de");
			break;
	}
	
	Globals::prefs->setMaximizedWindow(ui.checkBoxStartMaximized->isChecked());
	Globals::prefs->setStartFullscreen(ui.checkBoxStartFullscreen->isChecked());
	Globals::prefs->setFileOrder(ui.comboBoxFileOrder->currentIndex());
	Globals::prefs->setLoopDir(ui.checkBoxLoopDir->isChecked());
	Globals::prefs->setDisplayMode(ui.comboBoxDisplayMode->currentIndex());
	Globals::prefs->setFitWindowWhenZoomed(ui.checkBoxFitWindowWhenZoomed->isChecked());
	Globals::prefs->setDisplayBackground(ui.spinBoxDisplayBackground->value());
	Globals::prefs->setZoomDelta(ui.spinBoxZoomDelta->value());
	Globals::prefs->setReverseWheel(ui.checkBoxReverseMouseWheel->isChecked());
	Globals::prefs->setClearClipboardOnExit(ui.checkBoxClearClipboard->isChecked());
	Globals::prefs->setClearRecentFilesOnExit(ui.checkBoxClearRecentFiles->isChecked());
	Globals::prefs->setUseFastSelector(ui.comboBoxSelector->currentIndex() != 0);
	Globals::prefs->setEnableToolbarShrinking(ui.checkBoxEnableToolbarShrinking->isChecked());
	Globals::prefs->setDisplayHigherQuality(ui.comboBoxDisplayQuality->currentIndex() != 0);
	
	Globals::prefs->setFullscreenHideCursor(ui.checkBoxFsHideCursor->isChecked());
	Globals::prefs->setFullscreenDisplayMode(ui.comboBoxFsDisplayMode->currentIndex());
	Globals::prefs->setFullscreenFileIndexIndicator(ui.checkBoxFsFileIndexIndicator->isChecked());
	Globals::prefs->setFullscreenFileNameIndicator(ui.checkBoxFsFileNameIndicator->isChecked());
	switch (ui.comboBoxFsIndicatorColor->currentIndex())
	{	
		case 0: Globals::prefs->setFullscreenIndicatorColor("darkGray");
			break;
		case 1: Globals::prefs->setFullscreenIndicatorColor("gray");
			break;
		case 2: Globals::prefs->setFullscreenIndicatorColor("lightGray");
			break;
		case 3: Globals::prefs->setFullscreenIndicatorColor("white");
			break;
		case 4: Globals::prefs->setFullscreenIndicatorColor("darkRed");
			break;
		case 5: Globals::prefs->setFullscreenIndicatorColor("red");
			break;
		case 6: Globals::prefs->setFullscreenIndicatorColor("darkGreen");
			break;
		default:
		case 7: Globals::prefs->setFullscreenIndicatorColor("green");
			break;
		case 8: Globals::prefs->setFullscreenIndicatorColor("darkBlue");
			break;
		case 9: Globals::prefs->setFullscreenIndicatorColor("blue");
			break;
	}
	Globals::prefs->setFullscreenIndicatorFontFamily(ui.fontComboBoxFsIndicatorFontFamily->currentFont().family());
	Globals::prefs->setFullscreenIndicatorFontSize(ui.spinBoxFsIndicatorFontSize->value());
	
	Globals::prefs->setUndoStackMinimumSteps(ui.spinBoxUsMinimumSteps->value());
	Globals::prefs->setUndoStackMemoryLimit(ui.spinBoxUsMemoryLimit->value());

	Globals::prefs->setThumbnailsCacheSize(ui.spinBoxThCacheSize->value());
	Globals::prefs->setThumbnailsPreloading(ui.checkBoxThPreloading->isChecked());
	Globals::prefs->setThumbnailsThreads(ui.comboBoxThMultithreading->currentIndex());
	Globals::prefs->setThumbnailsScrollSpeed(ui.spinBoxThScrollSpeed->value());
	
	QStringList extEditorList;
	extEditorList.append((QStringList() << ui.lineEditExtEditor1Cmd->text() << ui.lineEditExtEditor1Args->text()).join(";"));
	extEditorList.append((QStringList() << ui.lineEditExtEditor2Cmd->text() << ui.lineEditExtEditor2Args->text()).join(";"));
	extEditorList.append((QStringList() << ui.lineEditExtEditor3Cmd->text() << ui.lineEditExtEditor3Args->text()).join(";"));
	extEditorList.append((QStringList() << ui.lineEditExtEditor4Cmd->text() << ui.lineEditExtEditor4Args->text()).join(";"));
	Globals::prefs->setExternalEditors(extEditorList);
	
	saveFileFormatSettings();
}

QCheckBox * PreferencesDialog::addFileFormatChechbox(QTableWidget * tableWidget, int row, int column, QString moduleName, QStringList list, QString listName, QString extension)
{
	QCheckBox * chechbox = new QCheckBox();
	chechbox->setObjectName(moduleName + ";" + listName + ";" + extension);
	chechbox->setChecked(!(list.contains(extension)));
	QHBoxLayout * layout = new QHBoxLayout();
	layout->addWidget(chechbox);
	layout->setAlignment(chechbox, Qt::AlignCenter);
	layout->setContentsMargins(0, 0, 0, 0);
	QWidget * container = new QWidget();
	container->setLayout(layout);
	tableWidget->setCellWidget(row, column, container);
	fileFormatSettings.append(chechbox);
	return chechbox;
}

void PreferencesDialog::toggleFileFormatChechbox(QString prefix)
{
	for (QCheckBox * item : fileFormatSettings)
	{
		if (item->objectName().startsWith(prefix))
		{
			item->click();
		}
	}
}

void PreferencesDialog::createFileFormatSettings()
{
	QList<IObase*> ioModules = imageIO->getModules();
	for (IObase * item : ioModules)
	{
		ui.listWidgetFileFormatsMoudles->addItem(item->moduleName());
		QWidget * pageWidget = new QWidget;
		QVBoxLayout * pageLayout = new QVBoxLayout();
		QTableWidget * tableWidget = new QTableWidget();
		tableWidget->setRowCount(1);
		tableWidget->setColumnCount(4);
		
		tableWidget->setHorizontalHeaderLabels({tr("Extension"), tr("Open"), tr("Thumbnail"), tr("Save")});
		tableWidget->verticalHeader()->setVisible(false);
		
		int row = 0;
		QStringList inputFormats = item->getAllInputFormats();
		QStringList outputFormats = item->getAllOutputFormats();
		QStringList allFormats = inputFormats + outputFormats;
		allFormats.removeDuplicates();
		
		QString moduleName = "IOmodule" + item->moduleName() + "/extensions";
		
		QStringList blockedOpenList = Globals::prefs->fetchSpecificParameter(moduleName, "blockedOpen", QVariant(QStringList())).toStringList();
		QStringList blockedThumbnailList = Globals::prefs->fetchSpecificParameter(moduleName, "blockedThumbnail", QVariant(QStringList())).toStringList();
		QStringList blockedSaveList = Globals::prefs->fetchSpecificParameter(moduleName, "blockedSave", QVariant(QStringList())).toStringList();
		
		QPushButton * toggleOpenButton = new QPushButton(tr("Toggle"));
		QPushButton * toggleThumbnailButton = new QPushButton(tr("Toggle"));
		QPushButton * toggleSaveButton = new QPushButton(tr("Toggle"));
		QObject::connect(toggleOpenButton, &QPushButton::clicked, [=]() { toggleFileFormatChechbox(moduleName + ";blockedOpen;"); });
		QObject::connect(toggleThumbnailButton, &QPushButton::clicked, [=]() { toggleFileFormatChechbox(moduleName + ";blockedThumbnail;"); });
		QObject::connect(toggleSaveButton, &QPushButton::clicked, [=]() { toggleFileFormatChechbox(moduleName + ";blockedSave;"); });
		tableWidget->setRowCount(row+1);
		tableWidget->setCellWidget(row, 1, toggleOpenButton);
		tableWidget->setCellWidget(row, 2, toggleThumbnailButton);
		tableWidget->setCellWidget(row, 3, toggleSaveButton);
		row += 1;
		
		for (const QString & ext :allFormats)
		{
			tableWidget->setRowCount(row+1);
			tableWidget->setCellWidget(row, 0, new QLabel(ext));
			
			if (inputFormats.contains(ext))
			{
				QCheckBox * openCheckBox = addFileFormatChechbox(tableWidget, row, 1, moduleName, blockedOpenList, "blockedOpen", ext);
				QCheckBox * thumbnailCheckBox = addFileFormatChechbox(tableWidget, row, 2, moduleName, blockedThumbnailList, "blockedThumbnail", ext);
				thumbnailCheckBox->setVisible(openCheckBox->isChecked());
				QObject::connect(openCheckBox, &QCheckBox::clicked, [=](bool checked) { thumbnailCheckBox->setVisible(checked); });
			}
			if (outputFormats.contains(ext))
			{
				addFileFormatChechbox(tableWidget, row, 3, moduleName, blockedSaveList, "blockedSave", ext);
			}
			row += 1;
		}
		
		pageLayout->addWidget(tableWidget);
		
		QHBoxLayout * extraFormatLayout = new QHBoxLayout();
		QLineEdit * extraFormats = new QLineEdit();
		extraFormats->setObjectName(moduleName);
		extraFormats->setPlaceholderText(tr("additional extensions, use semicolon as separator"));
		extraFormats->setToolTip(tr("additional extensions, use semicolon as separator"));
		extraFormats->setText(Globals::prefs->fetchSpecificParameter(moduleName, "extraOpen", QVariant(QStringList())).toStringList().join(";"));
		extraFormatLayout->addWidget(new QLabel(tr("Try to open:")));
		extraFormatLayout->addWidget(extraFormats);
		pageLayout->addItem(extraFormatLayout);
		extraFileFormatSettings.append(extraFormats);
		
		QHBoxLayout * unlistedLayout = new QHBoxLayout();
		QCheckBox * unlistedCheckBox = new QCheckBox("Try to open unlisted formats");
		unlistedCheckBox->setObjectName(moduleName);
		unlistedCheckBox->setChecked(Globals::prefs->fetchSpecificParameter(moduleName, "unlistedOpen", QVariant(false)).toBool());
		unlistedLayout->addWidget(unlistedCheckBox);
		pageLayout->addItem(unlistedLayout);
		unlistedFileFormatSettings.append(unlistedCheckBox);
		
		pageWidget->setLayout(pageLayout);
		ui.stackedWidgetFileFormats->addWidget(pageWidget);
	}
	ui.listWidgetFileFormatsMoudles->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	ui.listWidgetFileFormatsMoudles->setCurrentRow(0);
	ui.stackedWidgetFileFormats->setCurrentIndex(0);
	connect(ui.listWidgetFileFormatsMoudles, SIGNAL(currentRowChanged(int)), this, SLOT(changeFileFormatsTab(int)));
	
}

void PreferencesDialog::changeFileFormatsTab(int i)
{
	ui.stackedWidgetFileFormats->setCurrentIndex(i);
}

void PreferencesDialog::saveFileFormatSettings()
{
	QList<IObase*> ioModules = imageIO->getModules();
	for (IObase * item : ioModules)
	{
		QString moduleName = "IOmodule" + item->moduleName() + "/extensions";
		
		Globals::prefs->removeSpecificParameter(moduleName, "blockedOpen");
		Globals::prefs->removeSpecificParameter(moduleName, "blockedThumbnail");
		Globals::prefs->removeSpecificParameter(moduleName, "blockedSave");
	}
	for (const QCheckBox * item : fileFormatSettings)
	{
		QStringList names = item->objectName().split(';');
		if (names.length() != 3) continue;
		QString moduleName = names.at(0);
		QString listName = names.at(1);
		QString extension = names.at(2);
		if (moduleName.isEmpty()) continue;
		if (listName.isEmpty()) continue;
		if (extension.isEmpty()) continue;
		if (!(item->isChecked()))
		{
			QStringList blockedList = Globals::prefs->fetchSpecificParameter(moduleName, listName, QVariant(QStringList())).toStringList();
			blockedList += extension;
			Globals::prefs->storeSpecificParameter(moduleName, listName, blockedList);
		}
	}
	for (const QLineEdit * item : extraFileFormatSettings)
	{
		QString moduleName = item->objectName();
		if (moduleName.isEmpty()) continue;
		QStringList extraFormats = item->text().split(";");
		QStringList extraFormatsFiltered;
		for (QString & e : extraFormats)
		{
			QString f = e.simplified().toLower();
			f.replace(".", "");
			if (f.isEmpty()) continue;
			extraFormatsFiltered.append(f);
		}
		Globals::prefs->storeSpecificParameter(moduleName, "extraOpen", extraFormatsFiltered);
	}
	for (const QCheckBox * item : unlistedFileFormatSettings)
	{
		QString moduleName = item->objectName();
		if (moduleName.isEmpty()) continue;
		Globals::prefs->storeSpecificParameter(moduleName, "unlistedOpen", item->isChecked());
	}
}

