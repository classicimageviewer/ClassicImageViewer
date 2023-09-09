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


#ifndef EFFECTSDIALOG_H
#define EFFECTSDIALOG_H

#include <QDialog>
#include <QImage>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include "ui_effectsdialog.h"
#include "effects/effectHub.h"

class EffectsDialog : public QDialog
{
	Q_OBJECT
private: // typedefs
	typedef struct {
		QSpinBox * spinBox;
		QSlider * slider;
	} CoupledSpinboxSlider;
	typedef struct {
		QDoubleSpinBox * spinBox;
		QSlider * slider;
		double scale;
	} CoupledDoubleSpinboxSlider;
private: // variables
	Ui_EffectsDialog ui;
	QImage srcImg, originalImg;
	QImage sampleImg;
	QRect srcClickRect;
	EffectHub * effectHub;
	bool previewModeIsZoom;
	QList<CoupledSpinboxSlider> coupledSpinboxSliders;
	QList<CoupledDoubleSpinboxSlider> coupledDoubleSpinboxSliders;
	int effectId;
private: // functions
	void redrawSrc(QPoint zoomPos);
	void redrawDst();
	QImage applyEffectsOn(QImage image);
private slots:
	void srcClickedAt(QPoint pos);
	void effectChanged(int row);
	void coupledSpinboxValueChanged(int v);
	void coupledSliderValueChanged(int v);
	void coupledDoubleSpinboxValueChanged(double v);
	void coupledDoubleSliderValueChanged(int v);
	void integerValueChanged(int v);
	void doubleValueChanged(double v);
public:
	EffectsDialog(QImage image, QWidget * parent = NULL);
	~EffectsDialog();
	QImage applyEffects();

};

#endif //EFFECTSDIALOG_H
