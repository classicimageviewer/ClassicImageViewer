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


#ifndef HISTOGRAMDIALOG_H
#define HISTOGRAMDIALOG_H

#include <QDialog>
#include <QImage>
#include <QEnterEvent>
#include <vector>
#include "ui_histogramdialog.h"

class HistogramDialog : public QDialog
{
	Q_OBJECT
private:
	Ui_HistogramDialog ui;
	QGraphicsScene * graphicsSceneIntensity;
	QGraphicsScene * graphicsSceneRGB;
	uint64_t pixelCount;
	std::vector<uint64_t> intensityHistogram = std::vector<uint64_t>(256);
	std::vector<uint64_t> redHistogram = std::vector<uint64_t>(256);
	std::vector<uint64_t> greenHistogram = std::vector<uint64_t>(256);
	std::vector<uint64_t> blueHistogram = std::vector<uint64_t>(256);
	bool lockOpacity;
	bool drawLines;
	
	double determineScale(std::vector<uint64_t> & vector, int skipBins);
	void clearData(void);
	void drawHistogram(void);
	void drawLine(QGraphicsScene * scene, int xPrev, int x, double heightPrev, double height, double scale, QColor color, int zValue);
	void drawBar(QGraphicsScene * scene, int x, double height, QColor color, int zValue);
private slots:
	void printData(int v);
protected:
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
	void enterEvent(QEvent * event) override;
#else
	void enterEvent(QEnterEvent * event) override;
#endif
	void leaveEvent(QEvent * event) override;
public:
	HistogramDialog(QWidget * parent = NULL);
	~HistogramDialog();
	void processImage(QImage image);
};

#endif //HISTOGRAMDIALOG_H
