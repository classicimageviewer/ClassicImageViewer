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


#include "effectsdialog.h"
#include "globals.h"
#include <QDebug>
#include <QPainter>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QMessageBox>
#include <cmath>
#include "dialogs/textinputdialog.h"


EffectsDialog::EffectsDialog(QImage image, QString singleEffect, QMap<QString, QVariant> intialConfig, QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);

	coupledSpinboxSliders = QList<CoupledSpinboxSlider>();
	coupledDoubleSpinboxSliders = QList<CoupledDoubleSpinboxSlider>();

	originalImg = image;
	srcImg = image.scaled(320*Globals::scalingFactor, 320*Globals::scalingFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	sampleImg = srcImg;
	ui.labelSrc->setAlignment(Qt::AlignCenter);
	ui.labelDst->setAlignment(Qt::AlignCenter);
	QPixmap srcPix = QPixmap::fromImage(srcImg);
	srcPix.setDevicePixelRatio(Globals::scalingFactor);
	ui.labelSrc->setPixmap(srcPix);
	QSize pixSize = srcPix.size() / Globals::scalingFactor;
	srcClickRect = QRect(QPoint(160-pixSize.width()/2, 160-pixSize.height()/2), pixSize);
	
	ui.comboBoxPresets->setVisible(false);
	ui.pushButtonPresetAdd->setVisible(false);
	ui.pushButtonPresetRemove->setVisible(false);
	connect(ui.comboBoxPresets, SIGNAL(currentIndexChanged(int)), this, SLOT(presetChanged(int)));
	connect(ui.pushButtonPresetAdd, SIGNAL(clicked(bool)), this, SLOT(presetAdd(bool)));
	connect(ui.pushButtonPresetRemove, SIGNAL(clicked(bool)), this, SLOT(presetRemove(bool)));
	
	effectHub = new EffectHub();
	ui.listWidget->addItems(effectHub->getEffects());
	connect(ui.listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(effectChanged(int)));
	if ((singleEffect.length() > 0) && (effectHub->getEffects().contains(singleEffect)))
	{
		ui.listWidget->setCurrentRow(effectHub->getEffects().indexOf(singleEffect));
		ui.listWidget->hide();
		resize(1,1);	// minimum size
		setWindowTitle(singleEffect);
	}
	else
	{
		int id = Globals::prefs->fetchSpecificParameter("EffectsDialog", "Effect", QVariant(0)).toInt();
		if (intialConfig.contains("effectId"))
		{
			id = intialConfig["effectId"].toInt();
		}
		ui.listWidget->setCurrentRow(id);
	}
	
	if (intialConfig.contains("resetToDefaults"))
	{
		if (intialConfig["resetToDefaults"].toBool())
		{
			restoreDefaults(true);
		}
	}
	
	if (intialConfig.contains("effectParamsControlType") && intialConfig.contains("effectParamsParameterName") && intialConfig.contains("effectParamsParameterValue"))
	{
		QStringList effectParamsControlType = intialConfig["effectParamsControlType"].toStringList();
		QStringList effectParamsParameterName = intialConfig["effectParamsParameterName"].toStringList();
		QList<QVariant> effectParamsParameterValue = intialConfig["effectParamsParameterValue"].toList();
		
		int paramsLen = effectParamsParameterName.length();
		
		if ((effectParamsControlType.length() == paramsLen) && (effectParamsParameterValue.length() == paramsLen))
		{
			QList<EffectBase::ParameterCluster> params;
			for (int i = 0; i < paramsLen; i++)
			{
				EffectBase::ParameterCluster param;
				param.controlType = effectParamsControlType[i];
				param.parameterName = effectParamsParameterName[i];
				param.parameterValue = effectParamsParameterValue[i];
				params.append(param);
			}
			applyParameterList(params);
		}
	}
	
	connect(ui.labelSrc, SIGNAL(clickedAt(QPoint)), this, SLOT(srcClickedAt(QPoint)));
	connect(ui.buttonBox->button(QDialogButtonBox::RestoreDefaults), SIGNAL(clicked(bool)), this, SLOT(restoreDefaults(bool)));
}

EffectsDialog::~EffectsDialog()
{
	delete effectHub;
}

void EffectsDialog::srcClickedAt(QPoint pos)
{
	if (srcClickRect.contains(pos))
	{
		pos -= srcClickRect.topLeft();
		QPoint scaledPos = pos * Globals::scalingFactor;
		redrawSrc(scaledPos);
	}
}

void EffectsDialog::redrawSrc(QPoint zoomPos)
{
	if (previewModeIsZoom)
	{
		QPoint originalPos = QPoint((zoomPos.x()/(double)srcImg.width())*originalImg.width(), (zoomPos.y()/(double)srcImg.height())*originalImg.height());
		QSize preferredSize = QSize(320*Globals::scalingFactor, 320*Globals::scalingFactor);
		QRect preferredRect = QRect(QPoint(0,0), preferredSize);
		preferredRect.moveCenter(originalPos);
		QRect testRect = preferredRect & originalImg.rect();
		int area = testRect.width() * testRect.height();
		int bestArea = area;
		const int vecDx[] = { 0,  0,  1, -1,  1,  1, -1, -1};
		const int vecDy[] = { 1, -1,  0,  0,  1, -1,  1, -1};
		while (1)
		{
			bool updated = false;
			QRect bestRect;
			for (int i=0; i<8; i++)
			{
				int dx = vecDx[i];
				int dy = vecDy[i];
				testRect = preferredRect.translated(dx, dy) & originalImg.rect();
				area = testRect.width() * testRect.height();
				if (area > bestArea)
				{
					bestArea = area;
					bestRect = preferredRect.translated(dx, dy);
					updated = true;
				}
			}
			if (updated)
			{
				preferredRect = bestRect;
			}
			if (!updated) break;
		}
		preferredRect = preferredRect & originalImg.rect();
		sampleImg = originalImg.copy(preferredRect);
		redrawDst();
		
		QPoint actualPos = QPoint((preferredRect.center().x()/(double)originalImg.width())*srcImg.width(), (preferredRect.center().y()/(double)originalImg.height())*srcImg.height());
		
		QImage srcImgCopy = srcImg.copy();
		QPainter painter(&srcImgCopy);
		painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
		QPen pen = QPen(QBrush(), Globals::scalingFactor, Qt::SolidLine);
		pen.setColor(Qt::white);
		painter.setPen(pen);
		painter.setBrush(QBrush());
		painter.drawLine(actualPos.x(), 0, actualPos.x(), srcImg.height());
		painter.drawLine(0, actualPos.y(), srcImg.width(), actualPos.y());
		painter.end();
		
		QPixmap srcPix = QPixmap::fromImage(srcImgCopy);
		srcPix.setDevicePixelRatio(Globals::scalingFactor);
		ui.labelSrc->setPixmap(srcPix);
	}
	else
	{
		sampleImg = srcImg;
		redrawDst();
		QPixmap srcPix = QPixmap::fromImage(srcImg);
		srcPix.setDevicePixelRatio(Globals::scalingFactor);
		ui.labelSrc->setPixmap(srcPix);
	}
}

void EffectsDialog::redrawDst()
{
	QPixmap dstPix = QPixmap::fromImage(applyEffectsOn(sampleImg));
	dstPix.setDevicePixelRatio(Globals::scalingFactor);
	ui.labelDst->setPixmap(dstPix);
}

void EffectsDialog::effectChanged(int row)
{
	effectId = row;
	if (ui.listWidget->isVisible())
	{
		Globals::prefs->storeSpecificParameter("EffectsDialog", "Effect", effectId);
	}
	
	//disconnect
	for (CoupledSpinboxSlider elem : coupledSpinboxSliders)
	{
		disconnect(elem.spinBox, SIGNAL(valueChanged(int)), NULL, NULL);
		disconnect(elem.slider, SIGNAL(valueChanged(int)), NULL, NULL);
	}
	for (CoupledDoubleSpinboxSlider elem : coupledDoubleSpinboxSliders)
	{
		disconnect(elem.spinBox, SIGNAL(valueChanged(double)), NULL, NULL);
		disconnect(elem.slider, SIGNAL(valueChanged(int)), NULL, NULL);
	}
	coupledSpinboxSliders.clear();
	coupledDoubleSpinboxSliders.clear();
	
	//delete old items
	for (int i = ui.guestLayout->count()-1; i >= 0; i--)
	{
		int row, column, rowSpan, columnSpan;
		ui.guestLayout->getItemPosition(i, &row, &column, &rowSpan, &columnSpan);
		if (row > 0)
		{
			QLayoutItem * child = ui.guestLayout->takeAt(i);
			if (child)
			{
				delete child->widget();
				delete child;
			}
		}
	}
	
	int currentRow = 1;
	
	previewModeIsZoom = effectHub->previewModeIsZoom(effectId);
	QList<EffectBase::ParameterCluster> parameterList = effectHub->getListOfParameterClusters(effectId);
	for (EffectBase::ParameterCluster elem : parameterList)
	{
		if (elem.controlType == "spinbox")
		{
			ui.guestLayout->addWidget(new QLabel(elem.displayName), currentRow, 0);
			QSpinBox * sb = new QSpinBox();
			sb->setObjectName(elem.parameterName);
			sb->setMinimum(elem.parameterMinValue.toInt());
			sb->setMaximum(elem.parameterMaxValue.toInt());
			sb->setValue(elem.parameterValue.toInt());
			ui.guestLayout->addWidget(sb, currentRow, 1);
			connect(sb, SIGNAL(valueChanged(int)), this, SLOT(integerValueChanged(int)));
		} else
		if (elem.controlType == "doublespinbox")
		{
			ui.guestLayout->addWidget(new QLabel(elem.displayName), currentRow, 0);
			QDoubleSpinBox * sb = new QDoubleSpinBox();
			sb->setObjectName(elem.parameterName);
			sb->setMinimum(elem.parameterMinValue.toDouble());
			sb->setMaximum(elem.parameterMaxValue.toDouble());
			sb->setValue(elem.parameterValue.toDouble());
			ui.guestLayout->addWidget(sb, currentRow, 1);
			connect(sb, SIGNAL(valueChanged(double)), this, SLOT(doubleValueChanged(double)));
		} else
		if (elem.controlType == "slider")
		{
			ui.guestLayout->addWidget(new QLabel(elem.displayName), currentRow, 0);
			QSpinBox * sb = new QSpinBox();
			sb->setObjectName(elem.parameterName);
			sb->setMinimum(elem.parameterMinValue.toInt());
			sb->setMaximum(elem.parameterMaxValue.toInt());
			sb->setValue(elem.parameterValue.toInt());
			ui.guestLayout->addWidget(sb, currentRow, 1);
			QSlider * sl = new QSlider();
			sl->setOrientation(Qt::Horizontal);
			sl->setObjectName(elem.parameterName + "Slider");
			sl->setMinimum(elem.parameterMinValue.toInt());
			sl->setMaximum(elem.parameterMaxValue.toInt());
			sl->setValue(elem.parameterValue.toInt());
			ui.guestLayout->addWidget(sl, currentRow, 2);
			CoupledSpinboxSlider elem;
			elem.spinBox = sb;
			elem.slider = sl;
			coupledSpinboxSliders.append(elem);
			connect(sb, SIGNAL(valueChanged(int)), this, SLOT(coupledSpinboxValueChanged(int)));
			connect(sl, SIGNAL(valueChanged(int)), this, SLOT(coupledSliderValueChanged(int)));
		} else
		if (elem.controlType == "slider10")
		{
			ui.guestLayout->addWidget(new QLabel(elem.displayName), currentRow, 0);
			QDoubleSpinBox * sb = new QDoubleSpinBox();
			sb->setObjectName(elem.parameterName);
			sb->setDecimals(1);
			sb->setMinimum(elem.parameterMinValue.toDouble());
			sb->setMaximum(elem.parameterMaxValue.toDouble());
			sb->setValue(elem.parameterValue.toDouble());
			ui.guestLayout->addWidget(sb, currentRow, 1);
			QSlider * sl = new QSlider();
			sl->setOrientation(Qt::Horizontal);
			sl->setObjectName(elem.parameterName + "Slider");
			sl->setMinimum(elem.parameterMinValue.toDouble()*10);
			sl->setMaximum(elem.parameterMaxValue.toDouble()*10);
			sl->setValue(elem.parameterValue.toDouble()*10);
			ui.guestLayout->addWidget(sl, currentRow, 2);
			CoupledDoubleSpinboxSlider elem;
			elem.spinBox = sb;
			elem.slider = sl;
			elem.scale = 10;
			coupledDoubleSpinboxSliders.append(elem);
			connect(sb, SIGNAL(valueChanged(double)), this, SLOT(coupledDoubleSpinboxValueChanged(double)));
			connect(sl, SIGNAL(valueChanged(int)), this, SLOT(coupledDoubleSliderValueChanged(int)));
		} else
		if (elem.controlType == "slider100")
		{
			ui.guestLayout->addWidget(new QLabel(elem.displayName), currentRow, 0);
			QDoubleSpinBox * sb = new QDoubleSpinBox();
			sb->setObjectName(elem.parameterName);
			sb->setDecimals(2);
			sb->setMinimum(elem.parameterMinValue.toDouble());
			sb->setMaximum(elem.parameterMaxValue.toDouble());
			sb->setValue(elem.parameterValue.toDouble());
			ui.guestLayout->addWidget(sb, currentRow, 1);
			QSlider * sl = new QSlider();
			sl->setOrientation(Qt::Horizontal);
			sl->setObjectName(elem.parameterName + "Slider");
			sl->setMinimum(elem.parameterMinValue.toDouble()*100);
			sl->setMaximum(elem.parameterMaxValue.toDouble()*100);
			sl->setValue(elem.parameterValue.toDouble()*100);
			ui.guestLayout->addWidget(sl, currentRow, 2);
			CoupledDoubleSpinboxSlider elem;
			elem.spinBox = sb;
			elem.slider = sl;
			elem.scale = 100;
			coupledDoubleSpinboxSliders.append(elem);
			connect(sb, SIGNAL(valueChanged(double)), this, SLOT(coupledDoubleSpinboxValueChanged(double)));
			connect(sl, SIGNAL(valueChanged(int)), this, SLOT(coupledDoubleSliderValueChanged(int)));
		} else
		if (elem.controlType == "slider1000")
		{
			ui.guestLayout->addWidget(new QLabel(elem.displayName), currentRow, 0);
			QDoubleSpinBox * sb = new QDoubleSpinBox();
			sb->setObjectName(elem.parameterName);
			sb->setDecimals(3);
			sb->setMinimum(elem.parameterMinValue.toDouble());
			sb->setMaximum(elem.parameterMaxValue.toDouble());
			sb->setValue(elem.parameterValue.toDouble());
			ui.guestLayout->addWidget(sb, currentRow, 1);
			QSlider * sl = new QSlider();
			sl->setOrientation(Qt::Horizontal);
			sl->setObjectName(elem.parameterName + "Slider");
			sl->setMinimum(elem.parameterMinValue.toDouble()*1000);
			sl->setMaximum(elem.parameterMaxValue.toDouble()*1000);
			sl->setValue(elem.parameterValue.toDouble()*1000);
			ui.guestLayout->addWidget(sl, currentRow, 2);
			CoupledDoubleSpinboxSlider elem;
			elem.spinBox = sb;
			elem.slider = sl;
			elem.scale = 1000;
			coupledDoubleSpinboxSliders.append(elem);
			connect(sb, SIGNAL(valueChanged(double)), this, SLOT(coupledDoubleSpinboxValueChanged(double)));
			connect(sl, SIGNAL(valueChanged(int)), this, SLOT(coupledDoubleSliderValueChanged(int)));
		} else
		if (elem.controlType == "checkbox")
		{
			QCheckBox * cb = new QCheckBox(elem.displayName);
			cb->setObjectName(elem.parameterName);
			cb->setChecked(elem.parameterValue.toBool());
			ui.guestLayout->addWidget(cb, currentRow, 0, 1, -1);	// columnspan: all
			connect(cb, SIGNAL(stateChanged(int)), this, SLOT(integerValueChanged(int)));
		} else
		if (elem.controlType == "combobox")
		{
			ui.guestLayout->addWidget(new QLabel(elem.displayName), currentRow, 0);
			QComboBox * cb = new QComboBox();
			cb->setObjectName(elem.parameterName);
			cb->addItems(elem.parameterMinValue.toStringList());
			cb->setCurrentIndex(elem.parameterValue.toInt());
			ui.guestLayout->addWidget(cb, currentRow, 1);
			connect(cb, SIGNAL(currentIndexChanged(int)), this, SLOT(integerValueChanged(int)));
		} else
		if (elem.controlType == "textedit")
		{
			ui.guestLayout->addWidget(new QLabel(elem.displayName), currentRow, 0);
			QTextEdit * te = new QTextEdit();
			te->setObjectName(elem.parameterName);
			te->setText(elem.parameterValue.toString());
			te->setLineWrapMode(QTextEdit::NoWrap);
			ui.guestLayout->addWidget(te, currentRow, 1);
			connect(te, SIGNAL(textChanged()), this, SLOT(textValueChanged()));
		}
		//TODO more
		
		currentRow += 1;
	}
	
	ui.comboBoxPresets->setVisible(parameterList.size() > 0);
	ui.pushButtonPresetAdd->setVisible(parameterList.size() > 0);
	ui.pushButtonPresetRemove->setVisible(parameterList.size() > 0);
	updatePresets();
	
	srcClickedAt(QPoint(160, 160));
}

void EffectsDialog::coupledSpinboxValueChanged(int v)
{
	Q_UNUSED(v);
	for (CoupledSpinboxSlider elem : coupledSpinboxSliders)
	{
		elem.slider->blockSignals(true);
		elem.slider->setValue(elem.spinBox->value());
		elem.slider->blockSignals(false);
	}
	redrawDst();
}

void EffectsDialog::coupledSliderValueChanged(int v)
{
	Q_UNUSED(v);
	for (CoupledSpinboxSlider elem : coupledSpinboxSliders)
	{
		elem.spinBox->blockSignals(true);
		elem.spinBox->setValue(elem.slider->value());
		elem.spinBox->blockSignals(false);
	}
	redrawDst();
}

void EffectsDialog::coupledDoubleSpinboxValueChanged(double v)
{
	Q_UNUSED(v);
	for (CoupledDoubleSpinboxSlider elem : coupledDoubleSpinboxSliders)
	{
		elem.slider->blockSignals(true);
		elem.slider->setValue(std::round(elem.spinBox->value() * elem.scale));
		elem.slider->blockSignals(false);
	}
	redrawDst();
}

void EffectsDialog::coupledDoubleSliderValueChanged(int v)
{
	Q_UNUSED(v);
	for (CoupledDoubleSpinboxSlider elem : coupledDoubleSpinboxSliders)
	{
		elem.spinBox->blockSignals(true);
		elem.spinBox->setValue(elem.slider->value() / elem.scale);
		elem.spinBox->blockSignals(false);
	}
	redrawDst();
}

void EffectsDialog::integerValueChanged(int v)
{
	Q_UNUSED(v);
	redrawDst();
}

void EffectsDialog::doubleValueChanged(double v)
{
	Q_UNUSED(v);
	redrawDst();
}

void EffectsDialog::textValueChanged(void)
{
	redrawDst();
}

void EffectsDialog::restoreDefaults(bool b)
{
	Q_UNUSED(b);
	
	QList<EffectBase::ParameterCluster> parameterList = effectHub->getListOfParameterClusters(effectId);
	for (int i = 0; i < ui.guestLayout->count(); i++)
	{
		QLayoutItem * child = ui.guestLayout->itemAt(i);
		if (child != NULL)
		{
			QWidget * widget = child->widget();
			if (widget != NULL)
			{
				for (EffectBase::ParameterCluster elem : parameterList)
				{
					if ((elem.parameterName.length() > 0) && (elem.parameterName == widget->objectName()))
					{
						if ((elem.controlType == "spinbox") || (elem.controlType == "slider"))
						{
							((QSpinBox*)widget)->setValue(elem.parameterDefaultValue.toInt());
						} else
						if ((elem.controlType == "doublespinbox") || (elem.controlType == "slider10") || (elem.controlType == "slider100") || (elem.controlType == "slider1000"))
						{
							((QDoubleSpinBox*)widget)->setValue(elem.parameterDefaultValue.toDouble());
						} else
						if (elem.controlType == "checkbox")
						{
							((QCheckBox*)widget)->setChecked(elem.parameterDefaultValue.toBool());
						} else
						if (elem.controlType == "combobox")
						{
							((QComboBox*)widget)->setCurrentIndex(elem.parameterDefaultValue.toInt());
						} else
						if (elem.controlType == "textedit")
						{
							((QTextEdit*)widget)->setText(elem.parameterDefaultValue.toString());
						} else
						//TODO more
						{
							break;
						}
						break;
					}
				}
			}
		}
	}
	redrawDst();
	
	ui.comboBoxPresets->setCurrentIndex(0);
}

void EffectsDialog::presetChanged(int v)
{
	if (v == 0) return;
	v -= 1;
	QString moduleName = effectHub->getModuleName(effectId);
	QList<QVariant> presetList = Globals::prefs->fetchSpecificParameter("EffectsDialog", "PresetsOf" + moduleName, QList<QVariant>()).toList();
	QVariant preset = presetList.at(v);
	QList<QVariant> presetElements = preset.toList();
	QList<QVariant> presetSettings;
	if (presetElements.size() > 1)
	{
		presetSettings = presetElements.at(1).toList();
	}
	
	QList<EffectBase::ParameterCluster> parameterListOriginal = effectHub->getListOfParameterClusters(effectId);
	for (int i = 0; i < ui.guestLayout->count(); i++)
	{
		QLayoutItem * child = ui.guestLayout->itemAt(i);
		if (child != NULL)
		{
			QWidget * widget = child->widget();
			if (widget != NULL)
			{
				for (EffectBase::ParameterCluster elem : parameterListOriginal)
				{
					if ((elem.parameterName.length() > 0) && (elem.parameterName == widget->objectName()))
					{
						QVariant presetValue;
						for (int p = 0; p < presetSettings.size() / 2; p++)
						{
							if (elem.parameterName == presetSettings.at(p*2).toString() )
							{
								presetValue = presetSettings.at(p*2 + 1);
								break;
							}
						}
						if (!presetValue.isValid()) break;
						
						if ((elem.controlType == "spinbox") || (elem.controlType == "slider"))
						{
							((QSpinBox*)widget)->setValue(presetValue.toInt());
						} else
						if ((elem.controlType == "doublespinbox") || (elem.controlType == "slider10") || (elem.controlType == "slider100") || (elem.controlType == "slider1000"))
						{
							((QDoubleSpinBox*)widget)->setValue(presetValue.toDouble());
						} else
						if (elem.controlType == "checkbox")
						{
							((QCheckBox*)widget)->setChecked(presetValue.toBool());
						} else
						if (elem.controlType == "combobox")
						{
							((QComboBox*)widget)->setCurrentIndex(presetValue.toInt());
						} else
						if (elem.controlType == "textedit")
						{
							((QTextEdit*)widget)->setText(presetValue.toString());
						} else
						//TODO more
						{
							break;
						}
						break;
					}
				}
			}
		}
	}
}

void EffectsDialog::applyParameterList(QList<EffectBase::ParameterCluster> & parameterList)
{
	for (int i = 0; i < ui.guestLayout->count(); i++)
	{
		QLayoutItem * child = ui.guestLayout->itemAt(i);
		if (child != NULL)
		{
			QWidget * widget = child->widget();
			if (widget != NULL)
			{
				for (EffectBase::ParameterCluster elem : parameterList)
				{
					if ((elem.parameterName.length() > 0) && (elem.parameterName == widget->objectName()))
					{
						if ((elem.controlType == "spinbox") || (elem.controlType == "slider"))
						{
							((QSpinBox*)widget)->setValue(elem.parameterValue.toInt());
						} else
						if ((elem.controlType == "doublespinbox") || (elem.controlType == "slider10") || (elem.controlType == "slider100") || (elem.controlType == "slider1000"))
						{
							((QDoubleSpinBox*)widget)->setValue(elem.parameterValue.toDouble());
						} else
						if (elem.controlType == "checkbox")
						{
							((QCheckBox*)widget)->setChecked(elem.parameterValue.toBool());
						} else
						if (elem.controlType == "combobox")
						{
							((QComboBox*)widget)->setCurrentIndex(elem.parameterValue.toInt());
						} else
						if (elem.controlType == "textedit")
						{
							((QTextEdit*)widget)->setText(elem.parameterValue.toString());
						} else
						//TODO more
						{
							break;
						}
						break;
					}
				}
			}
		}
	}
	redrawDst();
	
	ui.comboBoxPresets->setCurrentIndex(0);
}

void EffectsDialog::presetAdd(bool b)
{
	Q_UNUSED(b);
	TextInputDialog * d = new TextInputDialog(tr("Preset name"), "");
	if (d->exec() == QDialog::Accepted)
	{
		QString newName = d->getText();
		if (newName.length() < 1)
		{
			QMessageBox::critical(this, tr("Error"), tr("Cannot create nameless preset."));
		}
		else
		{
			QString moduleName = effectHub->getModuleName(effectId);
			QList<QVariant> presetList = Globals::prefs->fetchSpecificParameter("EffectsDialog", "PresetsOf" + moduleName, QList<QVariant>()).toList();
			QList<QVariant> newPreset;
			newPreset.append(QVariant(newName));
			QList<QVariant> newSettings;
			
			QList<EffectBase::ParameterCluster> parameterListOriginal = effectHub->getListOfParameterClusters(effectId);
			for (int i = 0; i < ui.guestLayout->count(); i++)
			{
				QLayoutItem * child = ui.guestLayout->itemAt(i);
				if (child != NULL)
				{
					QWidget * widget = child->widget();
					if (widget != NULL)
					{
						for (EffectBase::ParameterCluster elem : parameterListOriginal)
						{
							if ((elem.parameterName.length() > 0) && (elem.parameterName == widget->objectName()))
							{
								QList<QVariant> parameter;
								parameter.append(QVariant(elem.parameterName));
								if ((elem.controlType == "spinbox") || (elem.controlType == "slider"))
								{
									parameter.append(QVariant(((QSpinBox*)widget)->value()));
								} else
								if ((elem.controlType == "doublespinbox") || (elem.controlType == "slider10") || (elem.controlType == "slider100") || (elem.controlType == "slider1000"))
								{
									parameter.append(QVariant(((QDoubleSpinBox*)widget)->value()));
								} else
								if (elem.controlType == "checkbox")
								{
									parameter.append(QVariant(((QCheckBox*)widget)->isChecked()));
								} else
								if (elem.controlType == "combobox")
								{
									parameter.append(QVariant(((QComboBox*)widget)->currentIndex()));
								} else
								if (elem.controlType == "textedit")
								{
									parameter.append(QVariant(((QTextEdit*)widget)->toPlainText()));
								} else
								//TODO more
								{
									break;
								}
								
								newSettings.append(parameter);
								break;
							}
						}
					}
				}
			}
			
			
			newPreset.append(QVariant(newSettings));
			presetList.append(QVariant(newPreset));
			Globals::prefs->storeSpecificParameter("EffectsDialog", "PresetsOf" + moduleName, presetList);
			
			updatePresets(ui.comboBoxPresets->count());
		}
	}
	delete d;
}

void EffectsDialog::presetRemove(bool b)
{
	Q_UNUSED(b);
	int removeIndex = ui.comboBoxPresets->currentIndex();
	if (removeIndex > 0)
	{
		removeIndex -= 1;
		QString moduleName = effectHub->getModuleName(effectId);
		QList<QVariant> presetList = Globals::prefs->fetchSpecificParameter("EffectsDialog", "PresetsOf" + moduleName, QList<QVariant>()).toList();
		presetList.removeAt(removeIndex);
		Globals::prefs->storeSpecificParameter("EffectsDialog", "PresetsOf" + moduleName, presetList);
		updatePresets(0);
	}
}

void EffectsDialog::updatePresets(int index)
{
	ui.comboBoxPresets->blockSignals(true);
	ui.comboBoxPresets->clear();
	ui.comboBoxPresets->addItem(tr("Presets"));
	
	QString moduleName = effectHub->getModuleName(effectId);
	QList<QVariant> presetList = Globals::prefs->fetchSpecificParameter("EffectsDialog", "PresetsOf" + moduleName, QList<QVariant>()).toList();
	for (QVariant & preset : presetList)
	{
		QList<QVariant> presetElements = preset.toList();
		QString presetName = "???";
		if (presetElements.size() > 0)
		{
			presetName = presetElements.at(0).toString();
		}
		ui.comboBoxPresets->addItem(presetName);
	}
	
	ui.comboBoxPresets->setCurrentIndex(index);
	ui.comboBoxPresets->blockSignals(false);
}

void EffectsDialog::getSelectedEffect(QString & name, int & effectId, QList<EffectBase::ParameterCluster> & parameterList)
{
	name = effectHub->getEffects().at(this->effectId);
	effectId = this->effectId;
	
	QList<EffectBase::ParameterCluster> parameterListOriginal = effectHub->getListOfParameterClusters(this->effectId);
	parameterList = QList<EffectBase::ParameterCluster>();
	for (int i = 0; i < ui.guestLayout->count(); i++)
	{
		QLayoutItem * child = ui.guestLayout->itemAt(i);
		if (child != NULL)
		{
			QWidget * widget = child->widget();
			if (widget != NULL)
			{
				for (EffectBase::ParameterCluster elem : parameterListOriginal)
				{
					if ((elem.parameterName.length() > 0) && (elem.parameterName == widget->objectName()))
					{
						EffectBase::ParameterCluster cluster;
						//cluster.parameterName = elem.parameterName;
						cluster = elem;
						if ((elem.controlType == "spinbox") || (elem.controlType == "slider"))
						{
							cluster.parameterValue = QVariant(((QSpinBox*)widget)->value());
						} else
						if ((elem.controlType == "doublespinbox") || (elem.controlType == "slider10") || (elem.controlType == "slider100") || (elem.controlType == "slider1000"))
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
						if (elem.controlType == "textedit")
						{
							cluster.parameterValue = QVariant(((QTextEdit*)widget)->toPlainText());
						} else
						//TODO more
						{
							break;
						}
						
						parameterList.append(cluster);
						break;
					}
				}
			}
		}
	}
}

QMap<QString, QVariant> EffectsDialog::getConfig()
{
	QMap<QString, QVariant> config;
	QString name;
	int effectId;
	QList<EffectBase::ParameterCluster> parameterList;
	getSelectedEffect(name, effectId, parameterList);
	config["effectName"] = name;
	config["effectId"] = effectId;
	QStringList effectParamsControlType;
	QStringList effectParamsParameterName;
	QList<QVariant> effectParamsParameterValue;
	for(EffectBase::ParameterCluster param : parameterList)
	{
		effectParamsControlType.append(param.controlType);
		effectParamsParameterName.append(param.parameterName);
		effectParamsParameterValue.append(param.parameterValue);
	}
	config["effectParamsControlType"] = effectParamsControlType;
	config["effectParamsParameterName"] = effectParamsParameterName;
	config["effectParamsParameterValue"] = effectParamsParameterValue;
	return config;
}

QImage EffectsDialog::applyEffectsOn(QImage image, bool saveParams)
{
	QString name;
	int id;
	QList<EffectBase::ParameterCluster> parameterList;
	getSelectedEffect(name, id, parameterList);
	if (saveParams)
	{
		effectHub->saveEffectParameters(id, parameterList);
	}
	return effectHub->applyEffect(id, image, parameterList);
}

QImage EffectsDialog::applyEffects()
{
	return applyEffectsOn(originalImg, true);
}


