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
#include <cmath>

EffectsDialog::EffectsDialog(QImage image, QString singleEffect, QWidget * parent) : QDialog(parent)
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
		ui.listWidget->setCurrentRow(Globals::prefs->fetchSpecificParameter("EffectsDialog", "Effect", QVariant(0)).toInt());
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
		}
		//TODO more
		
		currentRow += 1;
	}
	
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
						cluster.parameterName = elem.parameterName;
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

QImage EffectsDialog::applyEffectsOn(QImage image)
{
	QString name;
	int id;
	QList<EffectBase::ParameterCluster> parameterList;
	getSelectedEffect(name, id, parameterList);
	effectHub->saveEffectParameters(id, parameterList);
	return effectHub->applyEffect(id, image, parameterList);
}

QImage EffectsDialog::applyEffects()
{
	return applyEffectsOn(originalImg);
}


