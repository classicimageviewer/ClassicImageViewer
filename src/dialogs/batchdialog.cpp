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

#if defined(HAS_VIPS)
#include <vips.h>
#include <vips/operation.h>
#endif

#include "batchdialog.h"
#include "globals.h"
#include <QDebug>
#include <QFileDialog>
#include <QStandardPaths>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QMessageBox>
#include <QDir>
#include <algorithm>
#include <vector>
#include <cmath>
#include "lib/resizer.h"
#include "lib/autocolor.h"
#include "lib/sharpener.h"
#include "dialogs/effectsdialog.h"

BatchDialog::BatchDialog(QString indexedDirPath, QStringList indexedFiles, ImageIO * imageIO, QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);
	batchProcessRunning = false;
	this->indexedDirPath = indexedDirPath;
	this->indexedFiles = indexedFiles;
	this->imageIO = imageIO;
	ui.pushButtonAddFromCurrentDir->setEnabled(indexedFiles.length() > 0);
	batchInput = QStringList();
	
	ui.tableViewInput->installEventFilter(this);
	ui.tableViewEffects->installEventFilter(this);
	
	connect(ui.stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(pageChanged(int)));
	connect(ui.pushButtonClose, SIGNAL(clicked(bool)), this, SLOT(closeButtonPressed(bool)));
	connect(ui.pushButtonBack, SIGNAL(clicked(bool)), this, SLOT(prevPage(bool)));
	connect(ui.pushButtonNext, SIGNAL(clicked(bool)), this, SLOT(nextPage(bool)));
	
	connect(ui.pushButtonAddFromCurrentDir, SIGNAL(clicked(bool)), this, SLOT(inputAddFilesFromIndexedDir(bool)));
	connect(ui.pushButtonAddFiles, SIGNAL(clicked(bool)), this, SLOT(inputAddFiles(bool)));
	connect(ui.pushButtonRemove, SIGNAL(clicked(bool)), this, SLOT(inputRemoveFiles(bool)));
	connect(ui.pushButtonRemoveAll, SIGNAL(clicked(bool)), this, SLOT(inputRemoveAll(bool)));
	
	connect(&inputPreviewTimer, SIGNAL(timeout()), this, SLOT(inputPreviewTimerTimeout()));
	inputPreviewTimer.setInterval(50);
	inputPreviewTimer.start();
	
	ui.comboBoxResizeAlgorithm->addItems(Resizer::getListOfAlgorithms());
	connect(ui.comboBoxResizeMode, SIGNAL(currentIndexChanged(int)), this, SLOT(resizeModeChanged(int)));
	
	connect(ui.pushButtonAddEffect, SIGNAL(clicked(bool)), this, SLOT(addEffect(bool)));
	connect(ui.pushButtonRemoveEffect, SIGNAL(clicked(bool)), this, SLOT(removeEffects(bool)));
	
	connect(ui.pushButtonNewOutDir, SIGNAL(clicked(bool)), this, SLOT(newOutputDir(bool)));
	
	QStringList formatList;
	for (const QString & format : imageIO->getOutputFormats())
	{
		formatList.append(format.toUpper());
	}
	ui.comboBoxOutputFormat->addItems(formatList);
	connect(ui.comboBoxOutputFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(outputFormatChanged(int)));
	
	QImage img;
	if (Globals::scalingFactor >= 2)
	{
		img = QImage(":/pixmaps/pixmaps/batch_error@2x.png");
	}
	else
	{
		img = QImage(":/pixmaps/pixmaps/batch_error.png");
	}
	img.setDevicePixelRatio(Globals::scalingFactor);
	batchErrorPixmap = QPixmap::fromImage(img);
	batchErrorPixmap.setDevicePixelRatio(Globals::scalingFactor);
	
	if (Globals::scalingFactor >= 2)
	{
		img = QImage(":/pixmaps/pixmaps/batch_success@2x.png");
	}
	else
	{
		img = QImage(":/pixmaps/pixmaps/batch_success");
	}
	img.setDevicePixelRatio(Globals::scalingFactor);
	batchSuccessPixmap = QPixmap::fromImage(img);
	batchSuccessPixmap.setDevicePixelRatio(Globals::scalingFactor);
	
	batchEmptyPixmap = QPixmap(batchSuccessPixmap.size());
	batchEmptyPixmap.fill(Qt::transparent);
	batchEmptyPixmap.setDevicePixelRatio(Globals::scalingFactor);
	
	connect(&progressTimer, SIGNAL(timeout()), this, SLOT(progressTimerTimeout()));
	progressTimer.setInterval(100);
	
	ui.groupBoxCrop->setChecked(Globals::prefs->fetchSpecificParameter("BatchDialog", "crop", QVariant(false)).toBool());
	ui.spinBoxCropTop->setValue(Globals::prefs->fetchSpecificParameter("BatchDialog", "cropTop", QVariant(0)).toInt());
	ui.spinBoxCropRight->setValue(Globals::prefs->fetchSpecificParameter("BatchDialog", "cropRight", QVariant(0)).toInt());
	ui.spinBoxCropBottom->setValue(Globals::prefs->fetchSpecificParameter("BatchDialog", "cropBottom", QVariant(0)).toInt());
	ui.spinBoxCropLeft->setValue(Globals::prefs->fetchSpecificParameter("BatchDialog", "cropLeft", QVariant(0)).toInt());
	ui.groupBoxRotate->setChecked(Globals::prefs->fetchSpecificParameter("BatchDialog", "rotate", QVariant(false)).toBool());
	ui.comboBoxRotateDir->setCurrentIndex(Globals::prefs->fetchSpecificParameter("BatchDialog", "rotateDir", QVariant(0)).toInt());
	ui.comboBoxRotateCondition->setCurrentIndex(Globals::prefs->fetchSpecificParameter("BatchDialog", "rotateCondition", QVariant(0)).toInt());
	ui.checkBoxHflip->setChecked(Globals::prefs->fetchSpecificParameter("BatchDialog", "hFlip", QVariant(false)).toBool());
	ui.checkBoxVflip->setChecked(Globals::prefs->fetchSpecificParameter("BatchDialog", "vFlip", QVariant(false)).toBool());
	ui.checkBoxAutoColor->setChecked(Globals::prefs->fetchSpecificParameter("BatchDialog", "autoColor", QVariant(false)).toBool());
	ui.checkBoxSharpen->setChecked(Globals::prefs->fetchSpecificParameter("BatchDialog", "sharpen", QVariant(false)).toBool());
	ui.checkBoxGrayscale->setChecked(Globals::prefs->fetchSpecificParameter("BatchDialog", "grayscale", QVariant(false)).toBool());
	ui.checkBoxNegative->setChecked(Globals::prefs->fetchSpecificParameter("BatchDialog", "negative", QVariant(false)).toBool());
	ui.groupBoxResize->setChecked(Globals::prefs->fetchSpecificParameter("BatchDialog", "resize", QVariant(false)).toBool());
	ui.comboBoxResizeMode->setCurrentIndex(Globals::prefs->fetchSpecificParameter("BatchDialog", "resizeMode", QVariant(0)).toInt());
	ui.comboBoxResizeAlgorithm->setCurrentIndex(Globals::prefs->fetchSpecificParameter("BatchDialog", "resizeAlgo", QVariant(1)).toInt());
	ui.doubleSpinBoxResizeW->setValue(Globals::prefs->fetchSpecificParameter("BatchDialog", "resizeW", QVariant(128.0)).toDouble());
	ui.doubleSpinBoxResizeH->setValue(Globals::prefs->fetchSpecificParameter("BatchDialog", "resizeH", QVariant(128.0)).toDouble());
	ui.lineEditOutputDirectory->setText(Globals::prefs->fetchSpecificParameter("BatchDialog", "targetDirectory", QString("")).toString());
	ui.groupBoxRename->setChecked(Globals::prefs->fetchSpecificParameter("BatchDialog", "rename", QVariant(false)).toBool());
	ui.lineEditPattern->setText(Globals::prefs->fetchSpecificParameter("BatchDialog", "renamePattern", QString("@")).toString());
	ui.spinBoxCounterStart->setValue(Globals::prefs->fetchSpecificParameter("BatchDialog", "renameCounter", QVariant(1)).toInt());
	ui.spinBoxCounterIncrement->setValue(Globals::prefs->fetchSpecificParameter("BatchDialog", "renameCounterIncrement", QVariant(1)).toInt());
	ui.lineEditReplace1->setText(Globals::prefs->fetchSpecificParameter("BatchDialog", "renameReplace1", QString("")).toString());
	ui.lineEditWith1->setText(Globals::prefs->fetchSpecificParameter("BatchDialog", "renameWith1", QString("")).toString());
	ui.lineEditReplace2->setText(Globals::prefs->fetchSpecificParameter("BatchDialog", "renameReplace2", QString("")).toString());
	ui.lineEditWith2->setText(Globals::prefs->fetchSpecificParameter("BatchDialog", "renameWith2", QString("")).toString());
	ui.lineEditReplace3->setText(Globals::prefs->fetchSpecificParameter("BatchDialog", "renameReplace31", QString("")).toString());
	ui.lineEditWith3->setText(Globals::prefs->fetchSpecificParameter("BatchDialog", "renameWith3", QString("")).toString());
	ui.lineEditReplace4->setText(Globals::prefs->fetchSpecificParameter("BatchDialog", "renameReplace4", QString("")).toString());
	ui.lineEditWith4->setText(Globals::prefs->fetchSpecificParameter("BatchDialog", "renameWith4", QString("")).toString());
	ui.groupBoxConvert->setChecked(Globals::prefs->fetchSpecificParameter("BatchDialog", "convert", QVariant(true)).toBool());
	ui.spinBoxThreads->setValue(Globals::prefs->fetchSpecificParameter("BatchDialog", "threads", QVariant(0)).toInt());
	int preferredFormatIndex = imageIO->getOutputFormats().indexOf(Globals::prefs->fetchSpecificParameter("BatchDialog", "format", QString("invalidFormatString")).toString());
	if (preferredFormatIndex < 0)
	{
		preferredFormatIndex = 0;
	}
	ui.comboBoxOutputFormat->setCurrentIndex(preferredFormatIndex);
	
	ui.stackedWidget->setCurrentIndex(0);
	resizeModeChanged(ui.comboBoxResizeMode->currentIndex());
}

BatchDialog::~BatchDialog()
{
	delete ui.tableViewInput->selectionModel();
	delete ui.tableViewEffects->selectionModel();
	deleteOutputFormatParameters();
}

#define BATCHDIALOG_STACKWIDGET_LAST_CONFIG_PAGE	(2)

void BatchDialog::reject()
{
	int index = ui.stackedWidget->currentIndex();
	if (index <= BATCHDIALOG_STACKWIDGET_LAST_CONFIG_PAGE)
	{
		done(QDialog::Rejected);
	}
}

void BatchDialog::pageChanged(int page)
{
	ui.pushButtonBack->setEnabled((page > 0) && (page <= BATCHDIALOG_STACKWIDGET_LAST_CONFIG_PAGE));
	ui.pushButtonNext->setEnabled(page <= BATCHDIALOG_STACKWIDGET_LAST_CONFIG_PAGE);
	if (page < BATCHDIALOG_STACKWIDGET_LAST_CONFIG_PAGE)
	{
		ui.pushButtonNext->setText(tr("Next"));
	}
	else
	{
		ui.pushButtonNext->setText(tr("Start"));
	}
	if (page == 0)
	{
		inputPreviewTimer.start();
	}
	else
	{
		inputPreviewTimer.stop();
	}
}

void BatchDialog::closeButtonPressed(bool b)
{
	Q_UNUSED(b);
	int index = ui.stackedWidget->currentIndex();
	if (index <= BATCHDIALOG_STACKWIDGET_LAST_CONFIG_PAGE)
	{
		done(QDialog::Rejected);
	}
	else
	{
		if (batchProcessRunning)
		{
			abortBatchProcessing();
		}
		else
		{
			done(QDialog::Rejected);
		}
	}
}

void BatchDialog::prevPage(bool b)
{
	Q_UNUSED(b);
	int index = ui.stackedWidget->currentIndex();
	if (index <= 0) return;
	ui.stackedWidget->setCurrentIndex(index - 1);
}

void BatchDialog::nextPage(bool b)
{
	Q_UNUSED(b);
	int index = ui.stackedWidget->currentIndex();
	if (index > BATCHDIALOG_STACKWIDGET_LAST_CONFIG_PAGE) return;
	if (index == BATCHDIALOG_STACKWIDGET_LAST_CONFIG_PAGE)
	{
		if (batchInput.length() < 1)
		{
			QMessageBox::critical(this, tr("Error"), tr("Empty input file list."));
			return;
		}
		if (!(ui.groupBoxRename->isChecked() || ui.groupBoxConvert->isChecked()))
		{
			QMessageBox::critical(this, tr("Error"), tr("At least one of the Rename / Convert options have to be selected."));
			return;
		}
		QDir targetDir(ui.lineEditOutputDirectory->text());
		if ((ui.lineEditOutputDirectory->text().length() < 1) || (!targetDir.exists()))
		{
			QMessageBox::critical(this, tr("Error"), tr("No valid target directory provided."));
			return;
		}
	}
	ui.stackedWidget->setCurrentIndex(index + 1);
	if (index == BATCHDIALOG_STACKWIDGET_LAST_CONFIG_PAGE)
	{
		createOutputList();
		collectParameters();
		saveParameters();
		rebuildProgressTable(0);
		startBatchProcessing();
		ui.pushButtonClose->setText(tr("Abort"));
	}
}

void BatchDialog::inputAddFilesFromIndexedDir(bool b)
{
	Q_UNUSED(b);
	for (const QString & filePath : indexedFiles)
	{
		QString fullPath = indexedDirPath + "/" + filePath;
		if (!batchInput.contains(fullPath))
		{
			batchInput.append(fullPath);
		}
	}
	rebuildInputTable(0);
}

void BatchDialog::inputAddFiles(bool b)
{
	Q_UNUSED(b);
	
	QString baseDir = indexedDirPath;
	if (baseDir.isEmpty())
	{
		baseDir = Globals::prefs->getLastOpenedDir();
		if (baseDir.isEmpty())
		{
			baseDir = QStandardPaths::displayName(QStandardPaths::PicturesLocation);
		}
	}
	QString filters = QString(tr("All supported images"));
	filters.append(" (");
	for ( const QString &ext : imageIO->getInputFormats())
	{
		filters.append(QString("*.%1 *.%2 ").arg(ext, ext.toUpper()));
	}
	filters.append(");;");
	
	for ( const QString &ext : imageIO->getInputFormats())
	{
		filters.append(QString("%1 files (*.%2 *.%3);;").arg(ext.toUpper(), ext, ext.toUpper()));
	}
	
	filters.append(tr("All files"));
	filters.append(" (*)");
	
	QFileDialog dialog(this, tr("Open File"), baseDir, filters);
	dialog.setFileMode(QFileDialog::ExistingFiles);
	dialog.setOptions(QFileDialog::HideNameFilterDetails);
	if (dialog.exec())
	{
		QStringList newFiles = dialog.selectedFiles();
		for (const QString & fullPath : newFiles)
		{
			if (!batchInput.contains(fullPath))
			{
				batchInput.append(fullPath);
			}
		}
		rebuildInputTable(batchInput.length()-1);
	}
}

void BatchDialog::inputRemoveFiles(bool b)
{
	Q_UNUSED(b);
	QItemSelectionModel *select = ui.tableViewInput->selectionModel();
	std::vector<int> rowsToBeRemoved;
	if ((select != NULL) && (select->hasSelection()))
	{
		QModelIndexList selectedRows = select->selectedRows();
		for (const QModelIndex & modelIndex : selectedRows)
		{
			rowsToBeRemoved.push_back(modelIndex.row());
		}
		std::sort(rowsToBeRemoved.begin(), rowsToBeRemoved.end());
		if (rowsToBeRemoved.size() > 0)
		{
			for (int i=rowsToBeRemoved.size()-1; i>=0; i--)
			{
				batchInput.removeAt(rowsToBeRemoved.at(i));
			}
			int newIndex = rowsToBeRemoved.at(0);
			if (newIndex >= batchInput.length())
			{
				newIndex = batchInput.length() - 1;
			}
			rebuildInputTable(newIndex);
		}
	}
}

void BatchDialog::inputRemoveAll(bool b)
{
	Q_UNUSED(b);
	batchInput.clear();
	rebuildInputTable(0);
}

bool BatchDialog::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == ui.tableViewInput)
	{
		if (event->type() == QEvent::KeyPress)
		{
			QKeyEvent * keyEvent = (QKeyEvent*)event;
			if (keyEvent->key() == Qt::Key_Delete)
			{
				inputRemoveFiles(true);
				return true;
			}
			
			if ((keyEvent->key() == Qt::Key_PageUp) || (keyEvent->key() == Qt::Key_PageDown) || (keyEvent->key() == Qt::Key_Up) || (keyEvent->key() == Qt::Key_Down))
			{
				inputPreviewTimer.start();
			}
		}
	}
	if (watched == ui.tableViewEffects)
	{
		if (event->type() == QEvent::KeyPress)
		{
			QKeyEvent * keyEvent = (QKeyEvent*)event;
			if (keyEvent->key() == Qt::Key_Delete)
			{
				removeEffects(true);
				return true;
			}
		}
	}
	
	return QObject::eventFilter(watched, event);
}

void BatchDialog::rebuildInputTable(int index)
{
	QStandardItemModel * model = new QStandardItemModel();
	for (const QString & fullPath : batchInput)
	{
		QList<QStandardItem*> rowData;
		rowData.clear();
		rowData.append(new QStandardItem(fullPath));
		model->appendRow(rowData);
	}
	delete ui.tableViewInput->selectionModel();	// delete old model
	ui.tableViewInput->setModel(model);
	if (batchInput.length() > 0)
	{
		model->setHorizontalHeaderItem(0, new QStandardItem(tr("Files")));
		ui.tableViewInput->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	}
	if ((batchInput.length() > 0) && (index >= 0))
	{
		ui.tableViewInput->selectRow(index);
	}
	ui.tableViewInput->setWordWrap(false);
	ui.tableViewInput->setTextElideMode(Qt::ElideLeft);
}

void BatchDialog::inputPreviewTimerTimeout()
{
	if (ui.stackedWidget->currentIndex() != 0) return;
	loadPreview();
}

void BatchDialog::loadPreview()
{
	QItemSelectionModel *select = ui.tableViewInput->selectionModel();
	
	if ((select != NULL) && (select->hasSelection()))
	{
		QModelIndexList selectedRows = select->selectedRows();
		if (selectedRows.length() == 1)
		{
			QString newPreviewPath = batchInput.at(selectedRows.at(0).row());
			if (previewPath != newPreviewPath)
			{
				previewPath = newPreviewPath;
				ui.labelInputPreview->clear();
				QSize thumbnailSize = QSize(128, 128)*Globals::scalingFactor;
				QImage img = imageIO->loadThumbnail(previewPath, thumbnailSize);
				if (img.isNull())
				{
					ui.labelInputPreview->setText(tr("Error"));
				}
				else
				{
					img.setDevicePixelRatio(Globals::scalingFactor);
					QPixmap pixmap(thumbnailSize);
					pixmap.setDevicePixelRatio(Globals::scalingFactor);
					pixmap.fill(Qt::transparent);
					QPainter painter(&pixmap);
					painter.drawImage((thumbnailSize.width() - img.width())/(2*Globals::scalingFactor), (thumbnailSize.height() - img.height())/(2*Globals::scalingFactor), img);
					painter.end();
					ui.labelInputPreview->setPixmap(pixmap);
				}
			}
		}
		else
		{
			ui.labelInputPreview->clear();
			ui.labelInputPreview->setText(tr("Preview"));
		}
	}
	else
	{
		ui.labelInputPreview->clear();
		ui.labelInputPreview->setText(tr("Preview"));
	}
}

void BatchDialog::resizeModeChanged(int index)
{
	QString opLabel = QString(tr("pixels"));
	if (index == 4) opLabel = QString(tr("%"));
	if (index == 5) opLabel = QString(tr("x"));
	ui.labelResizeOpLabel->setText(opLabel);
	
	ui.doubleSpinBoxResizeW->setDecimals((index == 4) ? 2:0);
	ui.doubleSpinBoxResizeH->setDecimals((index == 4) ? 2:0);
	ui.doubleSpinBoxResizeH->setVisible(index == 5);
}

void BatchDialog::addEffect(bool b)
{
	Q_UNUSED(b);
	QApplication::setOverrideCursor(Qt::WaitCursor);
	if (batchInput.length() < 1)
	{
		QApplication::restoreOverrideCursor();
		QMessageBox::critical(this, tr("Error"), tr("Empty input file list."));
		return;
	}
	QImage img;
	if (previewPath.isEmpty())
	{
		img = imageIO->loadFile(batchInput.at(0));
	}
	else
	{
		img = imageIO->loadFile(previewPath);
	}
	if (img.isNull())
	{
		QApplication::restoreOverrideCursor();
		QMessageBox::critical(this, tr("Error"), tr("Cannot load image for preview."));
		return;
	}
	
	EffectHub * effectHub = new EffectHub();
	for (const BatchTypedefs::EffectEntry & effect : effectList)
	{
		img = effectHub->applyEffect(effect.effectId, img, effect.parameters);
	}
	delete effectHub;
	
	QApplication::restoreOverrideCursor();
	
	EffectsDialog * d = new EffectsDialog(img);
	if (d->exec() == QDialog::Accepted)
	{
		BatchTypedefs::EffectEntry entry;
		d->getSelectedEffect(entry.name, entry.effectId, entry.parameters);
		effectList.append(entry);
		rebuildEffectTable(effectList.length() - 1);
	}
	delete d;
}

void BatchDialog::removeEffects(bool b)
{
	Q_UNUSED(b);
	QItemSelectionModel *select = ui.tableViewEffects->selectionModel();
	std::vector<int> rowsToBeRemoved;
	if ((select != NULL) && (select->hasSelection()))
	{
		QModelIndexList selectedRows = select->selectedRows();
		for (const QModelIndex & modelIndex : selectedRows)
		{
			rowsToBeRemoved.push_back(modelIndex.row());
		}
		std::sort(rowsToBeRemoved.begin(), rowsToBeRemoved.end());
		if (rowsToBeRemoved.size() > 0)
		{
			for (int i=rowsToBeRemoved.size()-1; i>=0; i--)
			{
				effectList.removeAt(rowsToBeRemoved.at(i));
			}
			int newIndex = rowsToBeRemoved.at(0);
			if (newIndex >= effectList.length())
			{
				newIndex = effectList.length() - 1;
			}
			rebuildEffectTable(newIndex);
		}
	}
}

void BatchDialog::rebuildEffectTable(int index)
{
	QStandardItemModel * model = new QStandardItemModel();
	for (const BatchTypedefs::EffectEntry & effect : effectList)
	{
		QList<QStandardItem*> rowData;
		rowData.clear();
		rowData.append(new QStandardItem(effect.name));
		model->appendRow(rowData);
	}
	delete ui.tableViewEffects->selectionModel();	// delete old model
	ui.tableViewEffects->setModel(model);
	if (effectList.length() > 0)
	{
		model->setHorizontalHeaderItem(0, new QStandardItem(tr("Effects")));
		ui.tableViewEffects->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	}
	if ((effectList.length() > 0) && (index >= 0))
	{
		ui.tableViewEffects->selectRow(index);
	}
	ui.tableViewEffects->setWordWrap(false);
	ui.tableViewEffects->setTextElideMode(Qt::ElideRight);
}

void BatchDialog::newOutputDir(bool b)
{
	Q_UNUSED(b);
	
	QString baseDir = ui.lineEditOutputDirectory->text();
	if (baseDir.isEmpty())
	{
		baseDir = Globals::prefs->getLastOpenedDir();
		if (baseDir.isEmpty())
		{
			baseDir = QStandardPaths::displayName(QStandardPaths::PicturesLocation);
		}
	}
	
	QString newDir = QFileDialog::getExistingDirectory(this, tr("Select Target Directory"), baseDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (!newDir.isEmpty())
	{
		ui.lineEditOutputDirectory->setText(newDir);
	}
}

void BatchDialog::outputFormatChanged(int index)
{
	deleteOutputFormatParameters();
	QList<IObase::ParameterCluster> parameterList = imageIO->getListOfParameterClusters(imageIO->getOutputFormats().at(index));
	if (parameterList.length() > 0)
	{
		for (IObase::ParameterCluster elem : parameterList)
		{
			if (elem.controlType == "spinbox")
			{
				ui.verticalLayoutOutputFormatParameters->addWidget(new QLabel(elem.displayName));
				QSpinBox * sb = new QSpinBox();
				sb->setObjectName(elem.parameterName);
				sb->setMinimum(elem.parameterMinValue.toInt());
				sb->setMaximum(elem.parameterMaxValue.toInt());
				sb->setValue(elem.parameterValue.toInt());
				ui.verticalLayoutOutputFormatParameters->addWidget(sb);
			} else
			if (elem.controlType == "doublespinbox")
			{
				ui.verticalLayoutOutputFormatParameters->addWidget(new QLabel(elem.displayName));
				QDoubleSpinBox * sb = new QDoubleSpinBox();
				sb->setObjectName(elem.parameterName);
				sb->setMinimum(elem.parameterMinValue.toDouble());
				sb->setMaximum(elem.parameterMaxValue.toDouble());
				sb->setValue(elem.parameterValue.toDouble());
				ui.verticalLayoutOutputFormatParameters->addWidget(sb);
			} else
			if (elem.controlType == "checkbox")
			{
				QCheckBox * cb = new QCheckBox(elem.displayName);
				cb->setObjectName(elem.parameterName);
				cb->setChecked(elem.parameterValue.toBool());
				ui.verticalLayoutOutputFormatParameters->addWidget(cb);
			} else
			if (elem.controlType == "combobox")
			{
				ui.verticalLayoutOutputFormatParameters->addWidget(new QLabel(elem.displayName));
				QComboBox * cb = new QComboBox();
				cb->setObjectName(elem.parameterName);
				cb->addItems(elem.parameterMinValue.toStringList());
				cb->setCurrentIndex(elem.parameterValue.toInt());
				ui.verticalLayoutOutputFormatParameters->addWidget(cb);
			}
			//TODO more
		}
		ui.verticalLayoutOutputFormatParameters->addItem(new QSpacerItem(8,8, QSizePolicy::Minimum, QSizePolicy::Expanding));
	}
}

void BatchDialog::deleteOutputFormatParameters()
{
	QLayoutItem * child;
	while ((child = ui.verticalLayoutOutputFormatParameters->takeAt(0)) != NULL)
	{
		delete child->widget();
		delete child;
	}
}


void BatchDialog::createOutputList()
{
	outputList.clear();
	outputStatusList.clear();
	QString format = imageIO->getOutputFormats().at(ui.comboBoxOutputFormat->currentIndex());
	
	unsigned long long int counter = ui.spinBoxCounterStart->value();
	counter += 10000000000000000ULL;	// ensure non-negative value
	int increment = ui.spinBoxCounterIncrement->value();
	
	for (const QString & inputPath : batchInput)
	{
		QFileInfo info(inputPath);
		QString fileName, suffix;
		fileName = info.completeBaseName();
		
		if (ui.groupBoxConvert->isChecked())
		{
			suffix = format;
		}
		else
		{
			suffix = info.suffix();
		}
		if (suffix.length())
		{
			suffix = "." + suffix;
		}
		
		if (ui.groupBoxRename->isChecked())
		{
			QString pattern = ui.lineEditPattern->text();
			pattern.replace("@", fileName);
			
			int lastHashPos = -1;
			unsigned long long int counterCopy = counter;
			while ((lastHashPos = pattern.lastIndexOf("#", -1)) >= 0)
			{
				unsigned int digit = counterCopy % 10;
				QString digitString = QString("%1").arg(digit);
				pattern.replace(lastHashPos, 1, digitString);
				counterCopy /= 10;
			}
			counter += increment;
			fileName = pattern;
			
			if (ui.lineEditReplace1->text().length() > 0)
			{
				fileName.replace(ui.lineEditReplace1->text(), ui.lineEditWith1->text());
			}
			if (ui.lineEditReplace2->text().length() > 0)
			{
				fileName.replace(ui.lineEditReplace2->text(), ui.lineEditWith2->text());
			}
			if (ui.lineEditReplace3->text().length() > 0)
			{
				fileName.replace(ui.lineEditReplace3->text(), ui.lineEditWith3->text());
			}
			if (ui.lineEditReplace4->text().length() > 0)
			{
				fileName.replace(ui.lineEditReplace4->text(), ui.lineEditWith4->text());
			}
		}
		outputList.append(ui.lineEditOutputDirectory->text() + "/" + fileName + suffix);
		outputStatusList.append(QString());
	}
}

void BatchDialog::collectParameters()
{
	conversionParameters.crop = ui.groupBoxCrop->isChecked();
	conversionParameters.cropTop = ui.spinBoxCropTop->value();
	conversionParameters.cropRight = ui.spinBoxCropRight->value();
	conversionParameters.cropBottom = ui.spinBoxCropBottom->value();
	conversionParameters.cropLeft = ui.spinBoxCropLeft->value();
	conversionParameters.rote = ui.groupBoxRotate->isChecked();
	conversionParameters.rotateDir = ui.comboBoxRotateDir->currentIndex();
	conversionParameters.rotateCondition = ui.comboBoxRotateCondition->currentIndex();
	conversionParameters.hFlip = ui.checkBoxHflip->isChecked();
	conversionParameters.vFlip = ui.checkBoxVflip->isChecked();
	conversionParameters.autoColor = ui.checkBoxAutoColor->isChecked();
	conversionParameters.sharpen = ui.checkBoxSharpen->isChecked();
	conversionParameters.grayscale = ui.checkBoxGrayscale->isChecked();
	conversionParameters.negative = ui.checkBoxNegative->isChecked();
	conversionParameters.resize = ui.groupBoxResize->isChecked();
	conversionParameters.resizeMode = ui.comboBoxResizeMode->currentIndex();
	conversionParameters.resizeAlgorithm = ui.comboBoxResizeAlgorithm->currentIndex();
	conversionParameters.resizeW = ui.doubleSpinBoxResizeW->value();
	conversionParameters.resizeH = ui.doubleSpinBoxResizeH->value();
	conversionParameters.effectList = effectList;
	
	conversionParameters.format = imageIO->getOutputFormats().at(ui.comboBoxOutputFormat->currentIndex());
	conversionParameters.formatParameterList.clear();
	QList<IObase::ParameterCluster> parameterList = imageIO->getListOfParameterClusters(conversionParameters.format);
	for (int i=0; i<ui.verticalLayoutOutputFormatParameters->count(); i++)
	{
		QLayoutItem * child = ui.verticalLayoutOutputFormatParameters->itemAt(i);
		if (child != NULL)
		{
			QWidget * widget = child->widget();
			if (widget != NULL)
			{
				for (IObase::ParameterCluster elem : parameterList)
				{
					if ((elem.parameterName.length() > 0) && (elem.parameterName == widget->objectName()))
					{
						IObase::ParameterCluster cluster;
						cluster.parameterName = elem.parameterName;
						if (elem.controlType == "spinbox")
						{
							cluster.parameterValue = QVariant(((QSpinBox*)widget)->value());
						} else
						if (elem.controlType == "doublespinbox")
						{
							cluster.parameterValue = QVariant(((QDoubleSpinBox*)widget)->value());
						} else
						if (elem.controlType == "checkbox")
						{
							cluster.parameterValue = QVariant(((QCheckBox*)widget)->isChecked());
						} else
						if (elem.controlType == "combobox")
						{
							cluster.parameterValue = QVariant(((QComboBox*)widget)->currentIndex());
						} else
						//TODO more
						{
							break;
						}
						
						conversionParameters.formatParameterList.append(cluster);
						break;
					}
				}
			}
		}
	}
}

void BatchDialog::saveParameters()
{
	Globals::prefs->storeSpecificParameter("BatchDialog", "crop", ui.groupBoxCrop->isChecked());
	Globals::prefs->storeSpecificParameter("BatchDialog", "cropTop", ui.spinBoxCropTop->value());
	Globals::prefs->storeSpecificParameter("BatchDialog", "cropRight", ui.spinBoxCropRight->value());
	Globals::prefs->storeSpecificParameter("BatchDialog", "cropBottom", ui.spinBoxCropBottom->value());
	Globals::prefs->storeSpecificParameter("BatchDialog", "cropLeft", ui.spinBoxCropLeft->value());
	Globals::prefs->storeSpecificParameter("BatchDialog", "rotate", ui.groupBoxRotate->isChecked());
	Globals::prefs->storeSpecificParameter("BatchDialog", "rotateDir", ui.comboBoxRotateDir->currentIndex());
	Globals::prefs->storeSpecificParameter("BatchDialog", "rotateCondition", ui.comboBoxRotateCondition->currentIndex());
	Globals::prefs->storeSpecificParameter("BatchDialog", "hFlip", ui.checkBoxHflip->isChecked());
	Globals::prefs->storeSpecificParameter("BatchDialog", "vFlip", ui.checkBoxVflip->isChecked());
	Globals::prefs->storeSpecificParameter("BatchDialog", "autoColor", ui.checkBoxAutoColor->isChecked());
	Globals::prefs->storeSpecificParameter("BatchDialog", "sharpen", ui.checkBoxSharpen->isChecked());
	Globals::prefs->storeSpecificParameter("BatchDialog", "grayscale", ui.checkBoxGrayscale->isChecked());
	Globals::prefs->storeSpecificParameter("BatchDialog", "negative", ui.checkBoxNegative->isChecked());
	Globals::prefs->storeSpecificParameter("BatchDialog", "resize", ui.groupBoxResize->isChecked());
	Globals::prefs->storeSpecificParameter("BatchDialog", "resizeMode", ui.comboBoxResizeMode->currentIndex());
	Globals::prefs->storeSpecificParameter("BatchDialog", "resizeAlgo", ui.comboBoxResizeAlgorithm->currentIndex());
	Globals::prefs->storeSpecificParameter("BatchDialog", "resizeW", ui.doubleSpinBoxResizeW->value());
	Globals::prefs->storeSpecificParameter("BatchDialog", "resizeH", ui.doubleSpinBoxResizeH->value());
	Globals::prefs->storeSpecificParameter("BatchDialog", "targetDirectory", ui.lineEditOutputDirectory->text());
	Globals::prefs->storeSpecificParameter("BatchDialog", "rename", ui.groupBoxRename->isChecked());
	Globals::prefs->storeSpecificParameter("BatchDialog", "renamePattern", ui.lineEditPattern->text());
	Globals::prefs->storeSpecificParameter("BatchDialog", "renameCounter", ui.spinBoxCounterStart->value());
	Globals::prefs->storeSpecificParameter("BatchDialog", "renameCounterIncrement", ui.spinBoxCounterIncrement->value());
	Globals::prefs->storeSpecificParameter("BatchDialog", "renameReplace1", ui.lineEditReplace1->text());
	Globals::prefs->storeSpecificParameter("BatchDialog", "renameWith1", ui.lineEditWith1->text());
	Globals::prefs->storeSpecificParameter("BatchDialog", "renameReplace2", ui.lineEditReplace2->text());
	Globals::prefs->storeSpecificParameter("BatchDialog", "renameWith2", ui.lineEditWith2->text());
	Globals::prefs->storeSpecificParameter("BatchDialog", "renameReplace3", ui.lineEditReplace3->text());
	Globals::prefs->storeSpecificParameter("BatchDialog", "renameWith3", ui.lineEditWith3->text());
	Globals::prefs->storeSpecificParameter("BatchDialog", "renameReplace4", ui.lineEditReplace4->text());
	Globals::prefs->storeSpecificParameter("BatchDialog", "renameWith4", ui.lineEditWith4->text());
	Globals::prefs->storeSpecificParameter("BatchDialog", "convert", ui.groupBoxConvert->isChecked());
	Globals::prefs->storeSpecificParameter("BatchDialog", "threads", ui.spinBoxThreads->value());
	Globals::prefs->storeSpecificParameter("BatchDialog", "format", imageIO->getOutputFormats().at(ui.comboBoxOutputFormat->currentIndex()));

}

void BatchDialog::rebuildProgressTable(int index)
{
	QStandardItemModel * model = new QStandardItemModel();
	for (int i=0; i<outputList.length(); i++)
	{
		QList<QStandardItem*> rowData;
		rowData.clear();
		QString status = outputStatusList.at(i);
		QStandardItem * statusItem = new QStandardItem();
		if (status.isNull())
		{
			statusItem->setData(QVariant(batchEmptyPixmap), Qt::DecorationRole);
		} else
		if (status.isEmpty())
		{
			statusItem->setData(QVariant(batchSuccessPixmap), Qt::DecorationRole);
			statusItem->setToolTip(tr("done"));
		} else
		{
			statusItem->setData(QVariant(batchErrorPixmap), Qt::DecorationRole);
			statusItem->setToolTip(status);
		}
		rowData.append(statusItem);
		rowData.append(new QStandardItem(outputList.at(i)));
		model->appendRow(rowData);
	}
	delete ui.tableViewProgress->selectionModel();	// delete old model
	ui.tableViewProgress->setModel(model);
	if (outputList.length() > 0)
	{
		model->setHorizontalHeaderItem(0, new QStandardItem(""));
		ui.tableViewProgress->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
		model->setHorizontalHeaderItem(1, new QStandardItem(tr("Files")));
		ui.tableViewProgress->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	}
	if ((outputList.length() > 0) && (index >= 0))
	{
		ui.tableViewProgress->selectRow(index);
	}
	ui.tableViewProgress->setWordWrap(false);
	ui.tableViewProgress->setTextElideMode(Qt::ElideLeft);
	
}

void BatchDialog::progressTimerTimeout()
{
	bool running = false;
	int runningThreadCount = 0;
	for (int i=0; i<threadCount; i++)
	{
		if (workers[i]->isRunning())
		{
			running = true;
			runningThreadCount += 1;
		}
	}
	if (!running)
	{
		for (int i=0; i<threadCount; i++)
		{
			delete workers[i];
		}
		
		batchProcessRunning = false;
		ui.pushButtonClose->setText(tr("Close"));
		ui.pushButtonClose->setEnabled(true);
		ui.pushButtonBack->setEnabled(true);
		ui.labelInfo->clear();
		progressTimer.stop();
#if defined(HAS_VIPS)
		vips_concurrency_set(vipsRestore);
#endif
	}
	else
	{
		ui.labelInfo->setText(QString(tr("Threads: %1/%2")).arg(runningThreadCount).arg(threadCount));
	}
	
	int lastStatus = 0;
	double finished = 0;
	mutex.lock();
	int index = 0;
	for (const QString & status : outputStatusList)
	{
		if (!status.isNull())
		{
			lastStatus = index;
			finished += 1;
		}
		index += 1;
	}
	if (running)
	{
		lastStatus = -1;
	}
	rebuildProgressTable(lastStatus);
	ui.progressBar->setValue(100.0 * finished / outputStatusList.length());
	mutex.unlock();
}

void BatchDialog::startBatchProcessing()
{
	batchProcessRunning = true;
#if defined(HAS_VIPS)
	vipsRestore = vips_concurrency_get();
	vips_concurrency_set(1);
#endif
	threadCount = ui.spinBoxThreads->value();
	if (threadCount == 0)
	{
		threadCount = QThread::idealThreadCount();
	}
	if (threadCount > 64) threadCount = 64;
	if (threadCount < 1) threadCount = 1;
	BatchTypedefs::ConversionParameters * parameters = NULL;
	if (ui.groupBoxConvert->isChecked())
	{
		parameters = &conversionParameters;
	}
	else
	{
		threadCount = 1;	// rename with 1 thread
	}
	
	listHead = 0;
	for (int i=0; i<threadCount; i++)
	{
		workers[i] = new BatchWorker(&mutex, &listHead, &batchInput, &outputList, &outputStatusList, parameters);
	}
	progressTimer.start();
}

void BatchDialog::abortBatchProcessing()
{
	if (!batchProcessRunning) return;
	ui.pushButtonClose->setEnabled(false);
	for (int i=0; i<threadCount; i++)
	{
		workers[i]->setExitFlag();
	}
}


BatchWorker::BatchWorker(QMutex * mutex, int * listHead, QStringList * inputList, QStringList * outputList, QStringList * outputStatusList, BatchTypedefs::ConversionParameters * parameters, QObject *parent) : QThread(parent)
{
	this->mutex = mutex;
	this->listHead = listHead;
	this->inputList = inputList;
	this->outputList = outputList;
	this->outputStatusList = outputStatusList;
	this->parameters = parameters;
	exitFlag = false;
	imageIO = new ImageIO();
	effectHub = new EffectHub();
	start();
}

BatchWorker::~BatchWorker()
{
	wait();
	delete imageIO;
	delete effectHub;
}

void BatchWorker::run()
{
	QString status;
	int currentIndex = -1;
	while (1)
	{
		mutex->lock();
		if (currentIndex >= 0)
		{
			(*outputStatusList)[currentIndex] = status;
			status.clear();
		}
		if (exitFlag)
		{
			mutex->unlock();
			return;
		}
		if (*listHead >= inputList->length())
		{
			mutex->unlock();
			return;
		}
		currentIndex = *listHead;
		*listHead += 1;
		QString inputPath = inputList->at(currentIndex);
		QString outputPath = outputList->at(currentIndex);
		mutex->unlock();
		
		if (!QFile::exists(inputPath))
		{
			status = QString(tr("input"));
			continue;
		}
		if (QFile::exists(outputPath))
		{
			status = QString(tr("already exists"));
			continue;
		}
		
		if (parameters == NULL)	// rename only
		{
			if (!QFile::copy(inputPath, outputPath))
			{
				status = QString(tr("already exists"));
				continue;
			}
		}
		else	// full conversion
		{
			QImage img = imageIO->loadFile(inputPath);
			if (img.isNull())
			{
				status = QString(tr("cannot read"));
				continue;
			}
			if (parameters->crop)
			{
				QRect cropRect = img.rect();
				cropRect.adjust(parameters->cropLeft, parameters->cropTop, parameters->cropRight * -1, parameters->cropBottom * -1);
				if (!(cropRect.isValid() && (cropRect.width() > 0) && (cropRect.height() > 0)))
				{
					status = QString(tr("over-crop"));
					continue;
				}
				img = img.copy(cropRect);
			}
			if (parameters->rote)
			{
				bool condition = false;
				switch (parameters->rotateCondition)
				{
					case 0:
						condition = true;
						break;
					case 1:	//landscape
						condition = (img.width() > img.height());
						break;
					case 2:	//portrait
						condition = (img.height() > img.width());
						break;
				}
				if (condition)
				{
					switch (parameters->rotateDir)
					{
						case 0:
							img = img.transformed(QTransform().rotate(90.0), Qt::SmoothTransformation);
							break;
						case 1:
							img = img.transformed(QTransform().rotate(-90.0), Qt::SmoothTransformation);
							break;
						case 2:
							img = img.transformed(QTransform().rotate(180.0), Qt::SmoothTransformation);
							break;
					}
				}
			}
			if (parameters->hFlip)
			{
			#if QT_VERSION < QT_VERSION_CHECK(6,9,0)
				img = img.mirrored(true, false);
			#else
				img = img.flipped(Qt::Horizontal);
			#endif
			}
			if (parameters->vFlip)
			{
			#if QT_VERSION < QT_VERSION_CHECK(6,9,0)
				img = img.mirrored(false, true);
			#else
				img = img.flipped(Qt::Vertical);
			#endif
			}
			if (parameters->autoColor)
			{
				img = AutoColor::Adjust(img);
			}
			if (parameters->sharpen)
			{
				img = Sharpener::Sharpen(img, 0.05);
			}
			if (parameters->grayscale)
			{
				QImage tmp = img.convertToFormat(QImage::Format_Grayscale8).convertToFormat(QImage::Format_RGB32);
				if (img.hasAlphaChannel())
				{
					tmp.setAlphaChannel(img.convertToFormat(QImage::Format_Alpha8));
				}
				img = tmp;
			}
			if (parameters->negative)
			{
				img.invertPixels();
			}
			if (parameters->resize)
			{
				QSize newSize;
				int w = parameters->resizeW;
				int h = parameters->resizeH;
				double aspectRatio = (double)img.width() / (double)img.height();
				switch (parameters->resizeMode)
				{
					case 0:	// width
						newSize = QSize(w, std::round(w/aspectRatio));
						break;
					case 1:	// height
						newSize = QSize(std::round(w*aspectRatio), w);
						break;
					case 2:	// long side
						if (img.width() > img.height())
						{
							newSize = QSize(w, std::round(w/aspectRatio));
						}
						else
						{
							newSize = QSize(std::round(w*aspectRatio), w);
						}
						break;
					case 3:	// short side
						if (img.width() < img.height())
						{
							newSize = QSize(w, std::round(w/aspectRatio));
						}
						else
						{
							newSize = QSize(std::round(w*aspectRatio), w);
						}
						break;
					case 4:	// scale to
						newSize = img.size() * (parameters->resizeW / 100.0);
						break;
					case 5:	// new size
						newSize = QSize(w, h);
						break;
				}
				if (newSize.isEmpty())
				{
					status = QString(tr("resized to zero"));
					continue;
				}
				img = Resizer::Resize(img, newSize, (Resizer::Algorithm)(parameters->resizeAlgorithm));
			}
			
			for (const BatchTypedefs::EffectEntry & effect : parameters->effectList)
			{
				img = effectHub->applyEffect(effect.effectId, img, effect.parameters);
			}

			if (!imageIO->saveFile(outputPath, parameters->format, img, parameters->formatParameterList))
			{
				status = QString(tr("cannot write"));
				continue;
			}
		}
		status = QString("");	// done
	}
}

void BatchWorker::setExitFlag()
{
	exitFlag = true;
}


