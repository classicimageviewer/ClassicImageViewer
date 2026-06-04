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


#include "addtextdialog.h"
#include "globals.h"
#include <QDebug>
#include <QColorDialog>
#include "lib/imageOp.h"

AddTextDialog::AddTextDialog(QRect selection, QMap<QString, QVariant> intialConfig, QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);
	//setFixedSize(size());
	this->selection = selection;
	
	positionMode = qBound(0, Globals::prefs->fetchSpecificParameter("AddTextDialog", "positionMode", 1).toInt(), 2);
	
	if (selection.isNull())
	{
		ui.radioButtonPositionSelection->setEnabled(false);
		positionMode = qMin(1, positionMode);
	}
	else
	{
		ui.labelSelection->setText(QString(tr("X: %1, Y: %2, W: %3, H: %4")).arg(selection.x()).arg(selection.y()).arg(selection.width()).arg(selection.height()));
		positionMode = 0;
	}
	
	relativePosition = qBound(0, Globals::prefs->fetchSpecificParameter("AddTextDialog", "relativePosition", 0).toInt(), 8);
	
	ui.spinBoxPositionAbsoluteX->setValue(qMax(0, Globals::prefs->fetchSpecificParameter("AddTextDialog", "absolutePositionX", 0).toInt()));
	ui.spinBoxPositionAbsoluteY->setValue(qMax(0, Globals::prefs->fetchSpecificParameter("AddTextDialog", "absolutePositionY", 0).toInt()));
	ui.spinBoxPositionAbsoluteWidth->setValue(qMax(0, Globals::prefs->fetchSpecificParameter("AddTextDialog", "absolutePositionWidth", 0).toInt()));
	ui.spinBoxPositionAbsoluteHeight->setValue(qMax(0, Globals::prefs->fetchSpecificParameter("AddTextDialog", "absolutePositionHeight", 0).toInt()));
	
	ui.plainTextEdit->setPlainText(Globals::prefs->fetchSpecificParameter("AddTextDialog", "text", "").toString());
	
	ui.checkBoxUserTextNewLine1->setChecked(Globals::prefs->fetchSpecificParameter("AddTextDialog", "userTextnewLine1", false).toBool());
	ui.checkBoxUserTextNewLine2->setChecked(Globals::prefs->fetchSpecificParameter("AddTextDialog", "userTextnewLine2", false).toBool());
	ui.checkBoxUserTextNewLine3->setChecked(Globals::prefs->fetchSpecificParameter("AddTextDialog", "userTextnewLine3", false).toBool());
	ui.checkBoxUserTextNewLine4->setChecked(Globals::prefs->fetchSpecificParameter("AddTextDialog", "userTextnewLine4", false).toBool());
	
	ui.lineEditUserText1->setText(Globals::prefs->fetchSpecificParameter("AddTextDialog", "userText1", "").toString());
	ui.lineEditUserText2->setText(Globals::prefs->fetchSpecificParameter("AddTextDialog", "userText2", "").toString());
	ui.lineEditUserText3->setText(Globals::prefs->fetchSpecificParameter("AddTextDialog", "userText3", "").toString());
	ui.lineEditUserText4->setText(Globals::prefs->fetchSpecificParameter("AddTextDialog", "userText4", "").toString());
	
	ui.fontComboBox->setCurrentFont(Globals::prefs->fetchSpecificParameter("AddTextDialog", "font", QFont()).value<QFont>());
	fontColor = Globals::prefs->fetchSpecificParameter("AddTextDialog", "fontColor", QColor(Qt::black)).value<QColor>();
	ui.spinBoxFontSize->setValue(Globals::prefs->fetchSpecificParameter("AddTextDialog", "fontSize", 24).toInt());
	ui.pushButtonBold->setChecked(Globals::prefs->fetchSpecificParameter("AddTextDialog", "fontBold", false).toBool());
	ui.pushButtonItalic->setChecked(Globals::prefs->fetchSpecificParameter("AddTextDialog", "fontItalic", false).toBool());
	ui.comboBoxAlignmentH->setCurrentIndex(Globals::prefs->fetchSpecificParameter("AddTextDialog", "alignmentH", 0).toInt());
	ui.comboBoxAlignmentV->setCurrentIndex(Globals::prefs->fetchSpecificParameter("AddTextDialog", "alignmentV", 0).toInt());
	
	
	if (intialConfig.contains("positionMode"))
	{
		positionMode = intialConfig["positionMode"].toInt();
		positionMode = qMin(1, positionMode);
	}
	if (intialConfig.contains("relativePosition"))
	{
		relativePosition = intialConfig["relativePosition"].toInt();
	}
	if (intialConfig.contains("absolutePositionX"))
	{
		ui.spinBoxPositionAbsoluteX->setValue(qMax(0, intialConfig["absolutePositionX"].toInt()));
	}
	if (intialConfig.contains("absolutePositionY"))
	{
		ui.spinBoxPositionAbsoluteY->setValue(qMax(0, intialConfig["absolutePositionY"].toInt()));
	}
	if (intialConfig.contains("absolutePositionWidth"))
	{
		ui.spinBoxPositionAbsoluteWidth->setValue(qMax(0, intialConfig["absolutePositionWidth"].toInt()));
	}
	if (intialConfig.contains("absolutePositionHeight"))
	{
		ui.spinBoxPositionAbsoluteHeight->setValue(qMax(0, intialConfig["absolutePositionHeight"].toInt()));
	}
	if (intialConfig.contains("text"))
	{
		ui.plainTextEdit->setPlainText(intialConfig["text"].toString());
	}
	
	if (intialConfig.contains("font"))
	{
		ui.fontComboBox->setCurrentFont(intialConfig["font"].value<QFont>());
	}
	if (intialConfig.contains("fontColor"))
	{
		fontColor = intialConfig["fontColor"].value<QColor>();
	}
	if (intialConfig.contains("fontSize"))
	{
		ui.spinBoxFontSize->setValue(intialConfig["fontSize"].toInt());
	}
	if (intialConfig.contains("fontBold"))
	{
		ui.pushButtonBold->setChecked(intialConfig["fontBold"].toBool());
	}
	if (intialConfig.contains("fontItalic"))
	{
		ui.pushButtonItalic->setChecked(intialConfig["fontItalic"].toBool());
	}
	if (intialConfig.contains("alignmentH"))
	{
		ui.comboBoxAlignmentH->setCurrentIndex(intialConfig["alignmentH"].toInt());
	}
	if (intialConfig.contains("alignmentV"))
	{
		ui.comboBoxAlignmentV->setCurrentIndex(intialConfig["alignmentV"].toInt());
	}
	
	
	setPositionMode(positionMode);
	
	ui.radioButtonPositionRelativeTL->setChecked(relativePosition == 0);
	ui.radioButtonPositionRelativeTC->setChecked(relativePosition == 1);
	ui.radioButtonPositionRelativeTR->setChecked(relativePosition == 2);
	ui.radioButtonPositionRelativeLC->setChecked(relativePosition == 3);
	ui.radioButtonPositionRelativeC->setChecked( relativePosition == 4);
	ui.radioButtonPositionRelativeRC->setChecked(relativePosition == 5);
	ui.radioButtonPositionRelativeBL->setChecked(relativePosition == 6);
	ui.radioButtonPositionRelativeBC->setChecked(relativePosition == 7);
	ui.radioButtonPositionRelativeBR->setChecked(relativePosition == 8);
	
	connect(ui.radioButtonPositionSelection, &QRadioButton::clicked, this, [this](bool b) {
		Q_UNUSED(b);
		setPositionMode(0);
	});
	connect(ui.radioButtonPositionRelative, &QRadioButton::clicked, this, [this](bool b) {
		Q_UNUSED(b);
		setPositionMode(1);
	});
	connect(ui.radioButtonPositionAbsolute, &QRadioButton::clicked, this, [this](bool b) {
		Q_UNUSED(b);
		setPositionMode(2);
	});
	
	connect(ui.toolButtonAddUserText1, &QToolButton::clicked, this, [this](bool b) {
		Q_UNUSED(b);
		addUserText(ui.checkBoxUserTextNewLine1->isChecked(), ui.lineEditUserText1->text());
	});
	connect(ui.toolButtonAddUserText2, &QToolButton::clicked, this, [this](bool b) {
		Q_UNUSED(b);
		addUserText(ui.checkBoxUserTextNewLine2->isChecked(), ui.lineEditUserText2->text());
	});
	connect(ui.toolButtonAddUserText3, &QToolButton::clicked, this, [this](bool b) {
		Q_UNUSED(b);
		addUserText(ui.checkBoxUserTextNewLine3->isChecked(), ui.lineEditUserText3->text());
	});
	connect(ui.toolButtonAddUserText4, &QToolButton::clicked, this, [this](bool b) {
		Q_UNUSED(b);
		addUserText(ui.checkBoxUserTextNewLine4->isChecked(), ui.lineEditUserText4->text());
	});
	
	connect(ui.pushButtonColor, SIGNAL(clicked(bool)), this, SLOT(changeFontColor(bool)));
}

AddTextDialog::~AddTextDialog()
{
	
}

void AddTextDialog::setPositionMode(int mode)
{
	ui.radioButtonPositionSelection->setChecked(mode == 0);
	ui.radioButtonPositionRelative->setChecked(mode == 1);
	ui.radioButtonPositionAbsolute->setChecked(mode == 2);
	ui.framePositionRelative->setEnabled(mode == 1);
	ui.framePositionAbsolute->setEnabled(mode == 2);
	ui.comboBoxAlignmentH->setEnabled(mode != 1);
	ui.comboBoxAlignmentV->setEnabled(mode != 1);
}

void AddTextDialog::addUserText(bool newLine, QString userText)
{
	if (!newLine && userText.isEmpty()) return;
	QString text = ui.plainTextEdit->toPlainText();
	if (newLine) text += "\n";
	text += userText;
	ui.plainTextEdit->setPlainText(text);
}

void AddTextDialog::changeFontColor(bool b)
{
	Q_UNUSED(b);
	QColorDialog * d = new QColorDialog(fontColor);
	d->setOption(QColorDialog::ShowAlphaChannel);
	if (d->exec() == QDialog::Accepted)
	{
		fontColor = d->selectedColor();
	}
	delete d;
}

QImage AddTextDialog::addTextToImage(QImage img)
{
	QRect rect;
	QFont font = ui.fontComboBox->currentFont();
	font.setBold(ui.pushButtonBold->isChecked());
	font.setItalic(ui.pushButtonItalic->isChecked());
	font.setPixelSize(ui.spinBoxFontSize->value());
	int alignment = Qt::AlignLeft;
	
	if (ui.radioButtonPositionRelative->isChecked())
	{
		if (ui.radioButtonPositionRelativeTL->isChecked()) alignment = Qt::AlignLeft | Qt::AlignTop;
		if (ui.radioButtonPositionRelativeTC->isChecked()) alignment = Qt::AlignHCenter | Qt::AlignTop;
		if (ui.radioButtonPositionRelativeTR->isChecked()) alignment = Qt::AlignRight | Qt::AlignTop;
		if (ui.radioButtonPositionRelativeLC->isChecked()) alignment = Qt::AlignLeft | Qt::AlignVCenter;
		if (ui.radioButtonPositionRelativeC->isChecked())  alignment = Qt::AlignHCenter | Qt::AlignVCenter;
		if (ui.radioButtonPositionRelativeRC->isChecked()) alignment = Qt::AlignRight | Qt::AlignVCenter;
		if (ui.radioButtonPositionRelativeBL->isChecked()) alignment = Qt::AlignLeft | Qt::AlignBottom;
		if (ui.radioButtonPositionRelativeBC->isChecked()) alignment = Qt::AlignHCenter | Qt::AlignBottom;
		if (ui.radioButtonPositionRelativeBR->isChecked()) alignment = Qt::AlignRight | Qt::AlignBottom;
		rect = img.rect();
	}
	else
	{
		switch (ui.comboBoxAlignmentH->currentIndex())
		{
			default:
			case 0:
				alignment = Qt::AlignLeft;
				break;
			case 1:
				alignment = Qt::AlignHCenter;
				break;
			case 2:
				alignment = Qt::AlignRight;
				break;
		}
		switch (ui.comboBoxAlignmentV->currentIndex())
		{
			default:
			case 0:
				alignment |= Qt::AlignTop;
				break;
			case 1:
				alignment |= Qt::AlignVCenter;
				break;
			case 2:
				alignment |= Qt::AlignBottom;
				break;
		}
		if (ui.radioButtonPositionSelection->isChecked())
		{
			rect = selection;
		} else
		if (ui.radioButtonPositionAbsolute->isChecked())
		{
			rect = QRect(ui.spinBoxPositionAbsoluteX->value(), ui.spinBoxPositionAbsoluteY->value(), ui.spinBoxPositionAbsoluteWidth->value(), ui.spinBoxPositionAbsoluteHeight->value());
		}
	}
	
	return ImageOp::AddText(img, rect, ui.plainTextEdit->toPlainText(), alignment, font, fontColor);
}

void AddTextDialog::savePreferences()
{
	int positionMode = 0;
	if (ui.radioButtonPositionRelative->isChecked()) positionMode = 1;
	if (ui.radioButtonPositionAbsolute->isChecked()) positionMode = 2;
	if (positionMode > 0)
	{
		Globals::prefs->storeSpecificParameter("AddTextDialog", "positionMode", positionMode);
	}
	
	int relativePosition = 0;
	if (ui.radioButtonPositionRelativeTL->isChecked()) relativePosition = 0;
	if (ui.radioButtonPositionRelativeTC->isChecked()) relativePosition = 1;
	if (ui.radioButtonPositionRelativeTR->isChecked()) relativePosition = 2;
	if (ui.radioButtonPositionRelativeLC->isChecked()) relativePosition = 3;
	if (ui.radioButtonPositionRelativeC->isChecked())  relativePosition = 4;
	if (ui.radioButtonPositionRelativeRC->isChecked()) relativePosition = 5;
	if (ui.radioButtonPositionRelativeBL->isChecked()) relativePosition = 6;
	if (ui.radioButtonPositionRelativeBC->isChecked()) relativePosition = 7;
	if (ui.radioButtonPositionRelativeBR->isChecked()) relativePosition = 8;
	Globals::prefs->storeSpecificParameter("AddTextDialog", "relativePosition", relativePosition);
	
	Globals::prefs->storeSpecificParameter("AddTextDialog", "absolutePositionX", ui.spinBoxPositionAbsoluteX->value());
	Globals::prefs->storeSpecificParameter("AddTextDialog", "absolutePositionY", ui.spinBoxPositionAbsoluteY->value());
	Globals::prefs->storeSpecificParameter("AddTextDialog", "absolutePositionWidth", ui.spinBoxPositionAbsoluteWidth->value());
	Globals::prefs->storeSpecificParameter("AddTextDialog", "absolutePositionHeight", ui.spinBoxPositionAbsoluteHeight->value());
	
	Globals::prefs->storeSpecificParameter("AddTextDialog", "text", ui.plainTextEdit->toPlainText());
	
	Globals::prefs->storeSpecificParameter("AddTextDialog", "userTextnewLine1", ui.checkBoxUserTextNewLine1->isChecked());
	Globals::prefs->storeSpecificParameter("AddTextDialog", "userTextnewLine2", ui.checkBoxUserTextNewLine2->isChecked());
	Globals::prefs->storeSpecificParameter("AddTextDialog", "userTextnewLine3", ui.checkBoxUserTextNewLine3->isChecked());
	Globals::prefs->storeSpecificParameter("AddTextDialog", "userTextnewLine4", ui.checkBoxUserTextNewLine4->isChecked());
	
	Globals::prefs->storeSpecificParameter("AddTextDialog", "userText1", ui.lineEditUserText1->text());
	Globals::prefs->storeSpecificParameter("AddTextDialog", "userText2", ui.lineEditUserText2->text());
	Globals::prefs->storeSpecificParameter("AddTextDialog", "userText3", ui.lineEditUserText3->text());
	Globals::prefs->storeSpecificParameter("AddTextDialog", "userText4", ui.lineEditUserText4->text());
	
	Globals::prefs->storeSpecificParameter("AddTextDialog", "font", ui.fontComboBox->currentFont());
	Globals::prefs->storeSpecificParameter("AddTextDialog", "fontColor", fontColor);
	Globals::prefs->storeSpecificParameter("AddTextDialog", "fontSize", ui.spinBoxFontSize->value());
	Globals::prefs->storeSpecificParameter("AddTextDialog", "fontBold", ui.pushButtonBold->isChecked());
	Globals::prefs->storeSpecificParameter("AddTextDialog", "fontItalic", ui.pushButtonItalic->isChecked());
	Globals::prefs->storeSpecificParameter("AddTextDialog", "alignmentH", ui.comboBoxAlignmentH->currentIndex());
	Globals::prefs->storeSpecificParameter("AddTextDialog", "alignmentV", ui.comboBoxAlignmentV->currentIndex());
}

QMap<QString, QVariant> AddTextDialog::getConfig()
{
	QMap<QString, QVariant> config;
	
	config["positionMode"] = 0;
	if (ui.radioButtonPositionRelative->isChecked()) config["positionMode"] = 1;
	if (ui.radioButtonPositionAbsolute->isChecked()) config["positionMode"] = 2;
	
	config["relativePosition"] = 0;
	if (ui.radioButtonPositionRelativeTL->isChecked()) config["relativePosition"] = 0;
	if (ui.radioButtonPositionRelativeTC->isChecked()) config["relativePosition"] = 1;
	if (ui.radioButtonPositionRelativeTR->isChecked()) config["relativePosition"] = 2;
	if (ui.radioButtonPositionRelativeLC->isChecked()) config["relativePosition"] = 3;
	if (ui.radioButtonPositionRelativeC->isChecked())  config["relativePosition"] = 4;
	if (ui.radioButtonPositionRelativeRC->isChecked()) config["relativePosition"] = 5;
	if (ui.radioButtonPositionRelativeBL->isChecked()) config["relativePosition"] = 6;
	if (ui.radioButtonPositionRelativeBC->isChecked()) config["relativePosition"] = 7;
	if (ui.radioButtonPositionRelativeBR->isChecked()) config["relativePosition"] = 8;
	
	config["absolutePositionX"] = ui.spinBoxPositionAbsoluteX->value();
	config["absolutePositionY"] = ui.spinBoxPositionAbsoluteY->value();
	config["absolutePositionWidth"] = ui.spinBoxPositionAbsoluteWidth->value();
	config["absolutePositionHeight"] = ui.spinBoxPositionAbsoluteHeight->value();
	
	config["text"] = ui.plainTextEdit->toPlainText();
	
	config["font"] = ui.fontComboBox->currentFont();
	config["fontColor"] = fontColor;
	config["fontSize"] = ui.spinBoxFontSize->value();
	config["fontBold"] = ui.pushButtonBold->isChecked();
	config["fontItalic"] = ui.pushButtonItalic->isChecked();
	config["alignmentH"] = ui.comboBoxAlignmentH->currentIndex();
	config["alignmentV"] = ui.comboBoxAlignmentV->currentIndex();
	
	return config;
}

