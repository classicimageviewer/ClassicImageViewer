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


#include "macroconfigdialog.h"
#include "globals.h"
#include <omp.h>
#include <QDebug>
#include <QGroupBox>
#include <QPushButton>
#include <QKeyEvent>
#include <QHeaderView>
#include <QColorDialog>
#include <cmath>
#include "addborderdialog.h"
#include "padtosizedialog.h"
#include "effectsdialog.h"
#include "lib/resizer.h"

MacroConfigResizeDialog::MacroConfigResizeDialog(QMap<QString, QVariant> intialConfig, QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);
	
	ui.comboBoxResizeAlgorithm->addItems(Resizer::getListOfAlgorithms());
	
	int resizeMode = 0;
	if (intialConfig.contains("resizeMode"))
	{
		resizeMode = intialConfig["resizeMode"].toInt();
	}
	int resizeAlgorithm = 0;
	if (intialConfig.contains("resizeAlgorithm"))
	{
		resizeAlgorithm = intialConfig["resizeAlgorithm"].toInt();
	}
	double resizeW = 128.0;
	if (intialConfig.contains("resizeW"))
	{
		resizeW = intialConfig["resizeW"].toDouble();
	}
	double resizeH = 128.0;
	if (intialConfig.contains("resizeH"))
	{
		resizeH = intialConfig["resizeH"].toDouble();
	}

	ui.comboBoxResizeMode->setCurrentIndex(resizeMode);
	ui.comboBoxResizeAlgorithm->setCurrentIndex(resizeAlgorithm);
	ui.doubleSpinBoxResizeW->setValue(resizeW);
	ui.doubleSpinBoxResizeH->setValue(resizeH);
	
	connect(ui.comboBoxResizeMode, SIGNAL(currentIndexChanged(int)), this, SLOT(resizeModeChanged(int)));
	resizeModeChanged(ui.comboBoxResizeMode->currentIndex());
}

MacroConfigResizeDialog::~MacroConfigResizeDialog() {}

QMap<QString, QVariant> MacroConfigResizeDialog::getConfig()
{
	QMap<QString, QVariant> config;
	config["resizeMode"] = ui.comboBoxResizeMode->currentIndex();
	config["resizeAlgorithm"] = ui.comboBoxResizeAlgorithm->currentIndex();
	config["resizeW"] = ui.doubleSpinBoxResizeW->value();
	config["resizeH"] = ui.doubleSpinBoxResizeH->value();
	return config;
}

void MacroConfigResizeDialog::resizeModeChanged(int index)
{
	QString opLabel = QString(tr("pixels"));
	if (index == 4) opLabel = QString(tr("%"));
	if (index == 5) opLabel = QString(tr("x"));
	ui.labelResizeOpLabel->setText(opLabel);
	
	ui.doubleSpinBoxResizeW->setDecimals((index == 4) ? 2:0);
	ui.doubleSpinBoxResizeH->setDecimals((index == 4) ? 2:0);
	ui.doubleSpinBoxResizeH->setVisible(index == 5);
}


/////__________
/////


#define HSpacer		new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum)
#define VSpacer		new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding)

MacroConfigDialog::MacroConfigDialog(QImage inputImage, QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);
	
	macroEngine = new MacroEngine();
	if (inputImage.isNull())
	{
		this->inputImage = defaultImage();
	}
	else
	{
		this->inputImage = inputImage;
	}
	
	QTabBar *tabBar = ui.tabWidget->tabBar();
	for (int i=0; i<9; i++)
	{
		ui.tabWidget->setTabText(i, "");
		QLabel *label = new QLabel();
		label->setText(QString("%1").arg(i+1));
		tabBar->setTabButton(i, QTabBar::LeftSide, label);
		QWidget * tab = ui.tabWidget->widget(i);
		assert(tab);
		QVBoxLayout * layout = new QVBoxLayout(tab);
		QHBoxLayout * tmpHBoxLayout;
		QVBoxLayout * tmpVBoxLayout;
		QGroupBox * tmpGroupBox;
		
		// macro
		tmpGroupBox = new QGroupBox(tr("Macro"));
		tmpGroupBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
		tmpVBoxLayout = new QVBoxLayout(tmpGroupBox);
		tmpHBoxLayout = new QHBoxLayout();
		enabled[i] = new QCheckBox(tr("Enable"));
		QPushButton * resetBtn = new QPushButton(tr("Clear"));
		tmpHBoxLayout->addWidget(enabled[i]);
		tmpHBoxLayout->addWidget(resetBtn);
		tmpHBoxLayout->addItem(HSpacer);
		tmpVBoxLayout->addLayout(tmpHBoxLayout);
		tmpHBoxLayout = new QHBoxLayout();
		userNote[i] = new QLineEdit();
		userNote[i]->setPlaceholderText(tr("user notes"));
		tmpHBoxLayout->addWidget(userNote[i]);
		tmpVBoxLayout->addLayout(tmpHBoxLayout);
		
		connect(resetBtn, &QPushButton::clicked, this, [this, i]() {clearMacroSlot(i);});
		
		layout->addWidget(tmpGroupBox);
		
		// input
		tmpGroupBox = new QGroupBox(tr("Input"));
		tmpGroupBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
		tmpHBoxLayout = new QHBoxLayout(tmpGroupBox);
		input[i] = new QComboBox();
		input[i]->addItem(tr("Selection"));
		input[i]->addItem(tr("Entire image"));
		tmpHBoxLayout->addWidget(input[i]);
		tmpHBoxLayout->addItem(HSpacer);
		layout->addWidget(tmpGroupBox);
		
		// operations
		tmpGroupBox = new QGroupBox(tr("Operations"));
		tmpGroupBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
		tmpHBoxLayout = new QHBoxLayout(tmpGroupBox);
		
		tmpVBoxLayout = new QVBoxLayout();
		macroList[i] = new QTableView();
		tmpVBoxLayout->addWidget(macroList[i]);
		deleteBtn[i] = new QPushButton(tr("Delete"));
		tmpVBoxLayout->addWidget(deleteBtn[i]);
		tmpHBoxLayout->addLayout(tmpVBoxLayout);

		tmpVBoxLayout = new QVBoxLayout();
		opList[i] = new QTableView();
		tmpVBoxLayout->addWidget(opList[i]);
		addBtn[i] = new QPushButton(tr("Add"));
		tmpVBoxLayout->addWidget(addBtn[i]);
		tmpHBoxLayout->addLayout(tmpVBoxLayout);
		
		connect(deleteBtn[i], &QPushButton::clicked, this, [this, i]() {deleteBtnSlot(i);});
		connect(addBtn[i], &QPushButton::clicked, this, [this, i]() {addBtnSlot(i);});
		
		layout->addWidget(tmpGroupBox);
		
		// output
		tmpGroupBox = new QGroupBox(tr("Output"));
		tmpGroupBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
		tmpVBoxLayout = new QVBoxLayout(tmpGroupBox);
		tmpHBoxLayout = new QHBoxLayout();
		outputImage[i] = new QComboBox();
		outputImage[i]->addItem(tr("Replace image"));
		outputImage[i]->addItem(tr("Insert back into selection"));
		outputImage[i]->addItem(tr("No change"));
		tmpHBoxLayout->addWidget(outputImage[i]);
		tmpHBoxLayout->addItem(new QSpacerItem(20, 0, QSizePolicy::Minimum, QSizePolicy::Minimum));
		outputCopyToClipboard[i] = new QCheckBox(tr("Copy to clipboard"));
		tmpHBoxLayout->addWidget(outputCopyToClipboard[i]);
		tmpHBoxLayout->addItem(HSpacer);
		tmpVBoxLayout->addLayout(tmpHBoxLayout);
		
		layout->addWidget(tmpGroupBox);
		
		// layout done
		
		// macroList
		macroList[i]->setModel(new QStandardItemModel());
		macroList[i]->setSelectionBehavior(QAbstractItemView::SelectRows);
		macroList[i]->setSelectionMode(QAbstractItemView::SingleSelection);

		QHeaderView * header = macroList[i]->horizontalHeader();
		header->setVisible(false);
		header = macroList[i]->verticalHeader();
		header->setVisible(true);
		
		macroList[i]->installEventFilter(this); 
		
		// opList
		
		QStandardItemModel * model = new QStandardItemModel();
		
		populateOpList(model, tr("Rotate left"), 		MacroEngine::Op::RotateLeft);
		populateOpList(model, tr("Rotate right"), 		MacroEngine::Op::RotateRight);
		populateOpList(model, tr("Custom rotation"), 		MacroEngine::Op::RotateCustom);
		populateOpList(model, tr("Vertical flip"), 		MacroEngine::Op::FlipVertical);
		populateOpList(model, tr("Horizontal flip"), 		MacroEngine::Op::FlipHorizontal);
		populateOpList(model, tr("Shear"), 			MacroEngine::Op::Shear);
		populateOpList(model, tr("Resize"), 			MacroEngine::Op::Resize);
		populateOpList(model, tr("Add border"), 		MacroEngine::Op::AddBorder);
		populateOpList(model, tr("Pad to size"), 		MacroEngine::Op::PaddToSize);
		populateOpList(model, tr("Increase color depth"), 	MacroEngine::Op::ColorDepthIncrease);
		populateOpList(model, tr("Decrease color depth"), 	MacroEngine::Op::ColorDepthDecrease);
		populateOpList(model, tr("Grayscale"), 			MacroEngine::Op::Grayscale);
		populateOpList(model, tr("Negative"), 			MacroEngine::Op::Negative);
		populateOpList(model, tr("Adjust colors"), 		MacroEngine::Op::AdjustColors);
		populateOpList(model, tr("Auto color adjust"), 		MacroEngine::Op::AutoColorAdjust);
		populateOpList(model, tr("Sharpen"), 			MacroEngine::Op::Sharpen);
		populateOpList(model, tr("Effects"), 			MacroEngine::Op::Effects);
		populateOpList(model, tr("External tools"), 		MacroEngine::Op::ExternalTools);

		
		opList[i]->setModel(model);
		opList[i]->setSelectionBehavior(QAbstractItemView::SelectRows);
		opList[i]->setSelectionMode(QAbstractItemView::SingleSelection);

		header = opList[i]->horizontalHeader();
		header->setSectionResizeMode(0, QHeaderView::Stretch);
		header->setVisible(false);
		header = opList[i]->verticalHeader();
		header->setVisible(false);
		
		connect(opList[i], &QTableView::doubleClicked, this, [this, i](const QModelIndex &index) {
			addOpSlot(i, static_cast<MacroEngine::Op>(index.data(Qt::UserRole).toInt()));
		});
		
		// load prefs
		userNote[i]->setText(Globals::prefs->fetchSpecificParameter("MacroConfigDialog", QString("note%1").arg(i+1), "").toString());
		enabled[i]->setChecked(isMacroEnabled(i+1));
		input[i]->setCurrentIndex(macroInputConfig(i+1));
		outputCopyToClipboard[i]->setChecked(macroOutputClipboard(i+1));
		outputImage[i]->setCurrentIndex(macroOutputConfig(i+1));
		
		macro[i] = getMacro(i+1);
		if (!macro[i].isEmpty())
		{
			rebuildMacroList(i);
		}
	}
	
	ui.tabWidget->setCurrentIndex(Globals::prefs->fetchSpecificParameter("MacroConfigDialog", "lastTab", 0).toInt());
	
	QSize dialogSize = Globals::prefs->fetchSpecificParameter("MacroConfigDialog", "dialogSize", QSize()).toSize();
	if (dialogSize.isValid())
	{
		resize(dialogSize);
	}

}

MacroConfigDialog::~MacroConfigDialog()
{
	Globals::prefs->storeSpecificParameter("MacroConfigDialog", "lastTab", ui.tabWidget->currentIndex());
	Globals::prefs->storeSpecificParameter("MacroConfigDialog", "dialogSize", size());
	delete macroEngine;
}

bool MacroConfigDialog::eventFilter(QObject* watched, QEvent* event)
{
	for (int i=0; i<9; i++)
	{
		if (watched == macroList[i])
		{
			if (event->type() == QEvent::KeyPress)
			{
				QModelIndex index = macroList[i]->currentIndex();
				if (!index.isValid()) break;
				int row = index.row();
				if ((row < 0) || (row >= macro[i].size())) break;
				
				QKeyEvent * keyEvent = (QKeyEvent*)event;
				if (keyEvent->key() == Qt::Key_Delete)
				{
					macro[i].removeAt(row);
					if (row >= macro[i].size())
					{
						row = macro[i].size() - 1;
					}
					macroList[i]->selectRow(row);
					rebuildMacroList(i);
					return true;
				} else
				if ((keyEvent->modifiers() == Qt::ControlModifier) && (keyEvent->key() == Qt::Key_Up)) 
				{
					if (row <= 0) break;
					macro[i].swapItemsAt(row, row-1);
					macroList[i]->selectRow(row-1);
					rebuildMacroList(i);
					return true;
				} else
				if ((keyEvent->modifiers() == Qt::ControlModifier) && (keyEvent->key() == Qt::Key_Down)) 
				{
					if (row >= (macro[i].size() - 1)) break;
					macro[i].swapItemsAt(row, row+1);
					macroList[i]->selectRow(row+1);
					rebuildMacroList(i);
					return true;
				}
			}
			break;
		}
	}
	
	return QObject::eventFilter(watched, event);
}

void MacroConfigDialog::deleteBtnSlot(int page)
{
	QModelIndex index = macroList[page]->currentIndex();
	if (!index.isValid()) return;
	int row = index.row();
	if ((row < 0) || (row >= macro[page].size())) return;
	macro[page].removeAt(row);
	if (row >= macro[page].size())
	{
		row = macro[page].size() - 1;
	}
	macroList[page]->selectRow(row);
	rebuildMacroList(page);
}

void MacroConfigDialog::addBtnSlot(int page)
{
	QModelIndex index = opList[page]->currentIndex();
	if (!index.isValid()) return;
	MacroEngine::Op op = static_cast<MacroEngine::Op>(index.data(Qt::UserRole).toInt());
	addOpSlot(page, op);
}

void MacroConfigDialog::clearMacroSlot(int page)
{
	enabled[page]->setChecked(false);
	userNote[page]->clear();
	input[page]->setCurrentIndex(0);
	macro[page].clear();
	outputCopyToClipboard[page]->setChecked(false);
	outputImage[page]->setCurrentIndex(0);
	rebuildMacroList(page);
}

void MacroConfigDialog::populateOpList(QStandardItemModel * model, QString text, MacroEngine::Op op)
{
	// skip currently unsupported ops
	if (op == MacroEngine::Op::RotateCustom) return;
	if (op == MacroEngine::Op::Shear) return;
	if (op == MacroEngine::Op::ExternalTools) return;
	
	int opInt = static_cast<int>(op);
	QStandardItem * item = new QStandardItem(text);
	item->setData(opInt, Qt::UserRole);
	item->setEditable(false);
	model->appendRow(item);
	
	displayNameList[opInt] = text;
}

void MacroConfigDialog::addOpSlot(int page, MacroEngine::Op op)
{
	QMap<QString, QVariant> opConfig = configureOp(op, macro[page]);
	if (opConfig.isEmpty()) return;
	
	macro[page].append(opConfig);
	rebuildMacroList(page);
}

void MacroConfigDialog::rebuildMacroList(int page)
{
	int selectedRow = -1;
	QModelIndex index = macroList[page]->currentIndex();
	if (index.isValid())
	{
		selectedRow = index.row();
	}
	
	if (macroList[page]->selectionModel())
	{
		delete macroList[page]->selectionModel();	// delete old model
	}
	
	QStandardItemModel * model = new QStandardItemModel();
	
	for (const QMap<QString, QVariant> & map : macro[page])
	{
		QList<QStandardItem*> rowData;
		rowData.clear();
		QStandardItem * item = new QStandardItem(map["displayName"].toString());
		item->setEditable(false);
		rowData.append(item);
		rowData.append(new QStandardItem());		// button placeholder
		model->appendRow(rowData);
	}
	
	macroList[page]->setModel(model);
	if (macro[page].isEmpty()) return;
	
	for (int row = 0; row<model->rowCount(); row++)
	{
		if (macro[page][row].contains("opConfig"))
		{
			QPushButton * btn = new QPushButton("🔧");
			btn->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
			btn->setMaximumWidth(50);
			connect(btn, &QPushButton::clicked, this, [this, page, row](bool b) {
				Q_UNUSED(b);
				reconfigureOp(page, row);
			});
			macroList[page]->setIndexWidget(model->item(row,1)->index(), btn);
		}
		else
		{
			QLabel * lbl = new QLabel("");
			lbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
			lbl->setMaximumWidth(50);
			macroList[page]->setIndexWidget(model->item(row,1)->index(), lbl);
		}
	}
	
	macroList[page]->setSelectionBehavior(QAbstractItemView::SelectRows);
	macroList[page]->setSelectionMode(QAbstractItemView::SingleSelection);
	QHeaderView * header = macroList[page]->horizontalHeader();
	header->setSectionResizeMode(0, QHeaderView::Stretch);
	header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	
	if (selectedRow >= 0)
	{
		macroList[page]->selectRow(selectedRow);
	}
}

QMap<QString, QVariant> MacroConfigDialog::configureOp(MacroEngine::Op op, QList<QMap<QString, QVariant>> precedingMacro, QMap<QString, QVariant> initialConfig)
{
	QMap<QString, QVariant> opConfig;
	QMap<QString, QVariant> emptyConfig;
	QMap<QString, QVariant> initialOpConfig;
	if (initialConfig.contains("opConfig"))
	{
		initialOpConfig = initialConfig["opConfig"].toMap();
	}
	
	switch(op)
	{
		default:
			return emptyConfig;
		// operations without config
		case MacroEngine::Op::RotateLeft:
		case MacroEngine::Op::RotateRight:
		case MacroEngine::Op::FlipVertical:
		case MacroEngine::Op::FlipHorizontal:
		case MacroEngine::Op::ColorDepthIncrease:
		case MacroEngine::Op::Grayscale:
		case MacroEngine::Op::Negative:
		case MacroEngine::Op::AutoColorAdjust:
		case MacroEngine::Op::Sharpen:
			break;
		// operations with config
		case MacroEngine::Op::RotateCustom:
		case MacroEngine::Op::Shear:
		case MacroEngine::Op::ExternalTools:
			assert(0);	// these are currently not supported
			break;
		case MacroEngine::Op::Resize:
			{
				MacroConfigResizeDialog * d = new MacroConfigResizeDialog(initialOpConfig);
				if (d->exec() != QDialog::Accepted)
				{
					delete d;
					return emptyConfig;
				}
				opConfig["opConfig"] = d->getConfig();
				delete d;
			}
			break;
		case MacroEngine::Op::AddBorder:
			{
				AddBorderDialog * d = new AddBorderDialog(initialOpConfig);
				if (d->exec() != QDialog::Accepted)
				{
					delete d;
					return emptyConfig;
				}
				opConfig["opConfig"] = d->getConfig();
				delete d;
			}
			break;
		case MacroEngine::Op::PaddToSize:
			{
				PadToSizeDialog * d = new PadToSizeDialog(initialOpConfig);
				if (d->exec() != QDialog::Accepted)
				{
					delete d;
					return emptyConfig;
				}
				opConfig["opConfig"] = d->getConfig();
				delete d;
			}
			break;
		case MacroEngine::Op::ColorDepthDecrease:
			{
				QImage preprocessedImage = macroEngine->runMacro(inputImage, precedingMacro);
				EffectsDialog * d = new EffectsDialog(preprocessedImage, QString(tr("Color Quantization")), initialOpConfig);
				if (d->exec() != QDialog::Accepted)
				{
					delete d;
					return emptyConfig;
				}
				opConfig["opConfig"] = d->getConfig();
				delete d;
			}
			break;
		case MacroEngine::Op::AdjustColors:
			{
				if (initialOpConfig.isEmpty())
				{
					initialOpConfig["resetToDefaults"] = true;
				}
				QImage preprocessedImage = macroEngine->runMacro(inputImage, precedingMacro);
				EffectsDialog * d = new EffectsDialog(preprocessedImage, QString(tr("Color Adjust")), initialOpConfig);
				if (d->exec() != QDialog::Accepted)
				{
					delete d;
					return emptyConfig;
				}
				opConfig["opConfig"] = d->getConfig();
				delete d;
			}
			break;
		case MacroEngine::Op::Effects:
			{
				QImage preprocessedImage = macroEngine->runMacro(inputImage, precedingMacro);
				EffectsDialog * d = new EffectsDialog(preprocessedImage, QString(), initialOpConfig);
				if (d->exec() != QDialog::Accepted)
				{
					delete d;
					return emptyConfig;
				}
				opConfig["opConfig"] = d->getConfig();
				delete d;
			}
			break;
	}
	opConfig["displayName"] = translateOpToDisplayName(op);
	opConfig["opCode"] = static_cast<int>(op);
	
	return opConfig;
}

void MacroConfigDialog::reconfigureOp(int page, int row)
{
	QMap<QString, QVariant> opConfig = macro[page].at(row);
	if (!opConfig.contains("opCode")) return;
	if (!opConfig.contains("opConfig")) return;
	MacroEngine::Op op = static_cast<MacroEngine::Op>(opConfig["opCode"].toInt());
	
	QList<QMap<QString, QVariant>> precedingMacro;
	if (row > 0)
	{
		precedingMacro = macro[page].mid(0, row);
	}
	QMap<QString, QVariant> newConfig = configureOp(op, precedingMacro, opConfig);
	
	if (newConfig.isEmpty()) return;
	
	macro[page][row] = newConfig;
}

QString MacroConfigDialog::translateOpToDisplayName(MacroEngine::Op op)
{
	if (!displayNameList.contains(static_cast<int>(op))) return "?";
	return displayNameList[static_cast<int>(op)];
}

QImage MacroConfigDialog::defaultImage()
{
	int imgSize = 320*Globals::scalingFactor;
	QImage image = QImage(imgSize, imgSize, QImage::Format_RGB32);
	image.fill(Qt::black);
	
	omp_set_num_threads(Globals::getThreadCount());
	#pragma omp parallel for schedule(static, 1)
	for (int y = 0; y < image.height(); y++)
	{
		QRgb* row = reinterpret_cast<QRgb *>(image.scanLine(y));
		for (int x = 0; x < image.width(); x++)
		{
			int dx = x - imgSize/2;
			int dy = y - imgSize/2;
			double radius = std::sqrt(dx*dx + dy*dy);
			if (radius > imgSize/2) continue;
			
			double angle = std::fmod((std::atan2(dy, dx) + M_PI), 2.0*M_PI) / (2.0*M_PI);
			
			radius = imgSize/2 - radius;
			radius /= imgSize/2;

			QColor hslC;
			hslC.setHsvF(angle, 1.0, radius);
			QRgb rgbQ = hslC.toRgb().rgb();
			row[x] = qRgb(qRed(rgbQ), qGreen(rgbQ), qBlue(rgbQ));
		}
	}
	return image;
}


QList<QMap<QString, QVariant>> MacroConfigDialog::getMacro(int index)
{
	auto storedList = Globals::prefs->fetchSpecificParameter("MacroConfigDialog", QString("macro%1").arg(index), QList<QVariant>()).toList();
	QList<QMap<QString, QVariant>> macro;
	for (QVariant & e : storedList)
	{
		macro.append(e.toMap());
	}
	return macro;
}

bool MacroConfigDialog::isMacroEnabled(int index)
{
	return Globals::prefs->fetchSpecificParameter("MacroConfigDialog", QString("enabled%1").arg(index), false).toBool();
}

int MacroConfigDialog::macroInputConfig(int index)
{
	return Globals::prefs->fetchSpecificParameter("MacroConfigDialog", QString("input%1").arg(index), 0).toInt();
}

int MacroConfigDialog::macroOutputConfig(int index)
{
	return Globals::prefs->fetchSpecificParameter("MacroConfigDialog", QString("output%1").arg(index), 0).toInt();
}

bool MacroConfigDialog::macroOutputClipboard(int index)
{
	return Globals::prefs->fetchSpecificParameter("MacroConfigDialog", QString("clipboard%1").arg(index), false).toBool();
}


void MacroConfigDialog::savePreferences()
{
	for (int i=0; i<9; i++)
	{
		Globals::prefs->storeSpecificParameter("MacroConfigDialog", QString("note%1").arg(i+1), userNote[i]->text());
		Globals::prefs->storeSpecificParameter("MacroConfigDialog", QString("enabled%1").arg(i+1), enabled[i]->isChecked());
		Globals::prefs->storeSpecificParameter("MacroConfigDialog", QString("input%1").arg(i+1), input[i]->currentIndex());
		Globals::prefs->storeSpecificParameter("MacroConfigDialog", QString("clipboard%1").arg(i+1), outputCopyToClipboard[i]->isChecked());
		Globals::prefs->storeSpecificParameter("MacroConfigDialog", QString("output%1").arg(i+1), outputImage[i]->currentIndex());
	
		QList<QVariant> listToBeStored;
		for (QMap<QString, QVariant> & e : macro[i])
		{
			listToBeStored.append(QVariant(e));
		}
		Globals::prefs->storeSpecificParameter("MacroConfigDialog", QString("macro%1").arg(i+1), listToBeStored);
	}
}

