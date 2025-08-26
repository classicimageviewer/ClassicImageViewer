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

PreferencesDialog::PreferencesDialog(QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.comboBoxDisplayMode, SIGNAL(currentIndexChanged(int)), this, SLOT(displayModeChanged(int)));

	ui.comboBoxLocale->setCurrentIndex(0);
	QString userLocale = Globals::prefs->getUserLocale();
	ui.comboBoxLocale->addItem("English");
	if (userLocale == "en") ui.comboBoxLocale->setCurrentIndex(1);
	
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
}

