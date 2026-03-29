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


#include "histogramdialog.h"
#include <omp.h>
#include "globals.h"
#include <QDebug>
#include <QColorDialog>
#include <algorithm>

#define OPACITY		0.8

HistogramDialog::HistogramDialog(QWidget * parent) : QDialog(parent)
{
	ui.setupUi(this);
	setModal(false);
	setFixedSize(size());
	setMouseTracking(true);
	setWindowFlags(windowFlags() | Qt::WindowDoesNotAcceptFocus | Qt::Tool);
	setAttribute(Qt::WA_ShowWithoutActivating);
	graphicsSceneIntensity = new QGraphicsScene(this);
	graphicsSceneIntensity->setSceneRect(0, 0, 256, 128);
	ui.graphicsViewIntensity->setScene(graphicsSceneIntensity);
	ui.graphicsViewIntensity->scale(1.0, 1.0);
	graphicsSceneRGB = new QGraphicsScene(this);
	graphicsSceneRGB->setSceneRect(0, 0, 256, 128);
	ui.graphicsViewRGB->setScene(graphicsSceneRGB);
	ui.graphicsViewRGB->scale(1.0, 1.0);
	pixelCount = 0;
	connect(ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(printData(int)));
	
	lockOpacity = Globals::prefs->fetchSpecificParameter("HistogramDialog", "lockOpacity", QVariant(false)).toBool();
	drawLines = Globals::prefs->fetchSpecificParameter("HistogramDialog", "drawLines", QVariant(false)).toBool();
	
	QMenu* menu = new QMenu(ui.toolButton);
	QAction * lockOpacityAction = new QAction(tr("Lock opacity"));
	lockOpacityAction->setCheckable(true);
	lockOpacityAction->setChecked(lockOpacity);
	connect(lockOpacityAction, &QAction::toggled, this, [this](bool checked) {lockOpacity = checked;});
	menu->addAction(lockOpacityAction);
	QAction * drawLinesAction = new QAction(tr("Draw lines"));
	drawLinesAction->setCheckable(true);
	drawLinesAction->setChecked(drawLines);
	connect(drawLinesAction, &QAction::toggled, this, [this](bool checked) {drawLines = checked; drawHistogram();});
	menu->addAction(drawLinesAction);
	ui.toolButton->setPopupMode(QToolButton::InstantPopup);
	ui.toolButton->setMenu(menu);
	
	setWindowOpacity(lockOpacity ? 1.0 : OPACITY);
}

HistogramDialog::~HistogramDialog()
{
	Globals::prefs->storeSpecificParameter("HistogramDialog", "lockOpacity", lockOpacity);
	Globals::prefs->storeSpecificParameter("HistogramDialog", "drawLines", drawLines);
}


#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
void HistogramDialog::enterEvent(QEvent * event)
#else
void HistogramDialog::enterEvent(QEnterEvent * event)
#endif
{
	Q_UNUSED(event);
	setWindowOpacity(1);
	QApplication::setOverrideCursor(Qt::ArrowCursor);
}

void HistogramDialog::leaveEvent(QEvent * event)
{
	Q_UNUSED(event);
	setWindowOpacity(lockOpacity ? 1.0 : OPACITY);
	QApplication::restoreOverrideCursor();
}

void HistogramDialog::printData(int v)
{
	Q_UNUSED(v);
	int index = ui.horizontalSlider->value();
	
	ui.labelPixelCount->setText(QString(tr("Pixel count:")) + QString(" %1").arg(pixelCount));
	ui.labelIndex->setText(QString(tr("Index:")) + QString(" %1").arg(index));
	if (pixelCount > 0)
	{
		ui.labelGrayCount->setText(QString("%1").arg(intensityHistogram[index]));
		ui.labelGrayPercent->setText(QString("%1 %").arg(((100.0 * intensityHistogram[index]) / pixelCount), 0, 'f', 1));
		ui.labelRedCount->setText(QString("%1").arg(redHistogram[index]));
		ui.labelRedPercent->setText(QString("%1 %").arg(((100.0 * redHistogram[index]) / pixelCount), 0, 'f', 1));
		ui.labelGreenCount->setText(QString("%1").arg(greenHistogram[index]));
		ui.labelGreenPercent->setText(QString("%1 %").arg(((100.0 * greenHistogram[index]) / pixelCount), 0, 'f', 1));
		ui.labelBlueCount->setText(QString("%1").arg(blueHistogram[index]));
		ui.labelBluePercent->setText(QString("%1 %").arg(((100.0 * blueHistogram[index]) / pixelCount), 0, 'f', 1));
	}
	else
	{
		ui.labelGrayCount->setText("-");
		ui.labelGrayPercent->setText("-");
		ui.labelRedCount->setText("-");
		ui.labelRedPercent->setText("-");
		ui.labelGreenCount->setText("-");
		ui.labelGreenPercent->setText("-");
		ui.labelBlueCount->setText("-");
		ui.labelBluePercent->setText("-");
	}
}

void HistogramDialog::drawLine(QGraphicsScene * scene, int xPrev, int x, double heightPrev, double height, double scale, QColor color, int zValue)
{
	height = qBound(0.0, height * scale, 1.0);
	heightPrev = qBound(0.0, heightPrev * scale, 1.0);
	height = 127 - height * 127;
	heightPrev = 127 - heightPrev * 127;
	auto line = scene->addLine(QLineF(xPrev+1, heightPrev, x+1, height), QPen(color));
	line->setZValue(zValue);
}

void HistogramDialog::drawBar(QGraphicsScene * scene, int x, double height, QColor color, int zValue)
{
	height = qBound(0.0, height, 1.0);
	height = 127 - height * 127;
	auto line = scene->addLine(QLineF(x+1, 127, x+1, height), QPen(color));
	line->setZValue(zValue);
}

double HistogramDialog::determineScale(std::vector<uint64_t> & vector, int skipBins)
{
	std::vector<uint64_t> tmp = vector;
	std::sort(tmp.begin(), tmp.end());
	std::reverse(tmp.begin(), tmp.end());
	
	if (tmp[skipBins] == 0) return 1.0;
	double scale = pixelCount;
	scale /= tmp[skipBins];
	return scale;
}

void HistogramDialog::drawHistogram(void)
{
	graphicsSceneIntensity->clear();
	graphicsSceneRGB->clear();
	graphicsSceneIntensity->setBackgroundBrush(Qt::black);
	graphicsSceneRGB->setBackgroundBrush(Qt::black);
	
	if (pixelCount == 0) return;
	
	double scaleI = determineScale(intensityHistogram, 4);
	double scaleRGB = qMin( determineScale(redHistogram, 4),
			  qMin( determineScale(greenHistogram, 4),
				determineScale(blueHistogram, 4)
				));
	scaleI /= pixelCount;
	scaleRGB /= pixelCount;
	
	uint64_t cum = 0;
	for(int i=0; i<256; i++)
	{
		
		uint64_t prevCum = cum;
		cum += intensityHistogram[i];
		if (i == 0) prevCum = cum;
		double cumStart = prevCum;
		double cumEnd = cum;
		cumStart /= pixelCount;
		cumEnd /= pixelCount;
		auto line = graphicsSceneIntensity->addLine(QLineF(i+2, 127 - cumStart*127, i+2, 127 - cumEnd*127), QPen(QColor(255, 0, 96)));
		line->setZValue(1);
		
		if (drawLines)
		{
			int h = i - 1;
			if (h < 0) h = 0;
			drawLine(graphicsSceneIntensity, h, i, intensityHistogram[h], intensityHistogram[i], scaleI, Qt::white, 0);
			drawLine(graphicsSceneRGB, h, i, redHistogram[h], redHistogram[i], scaleRGB, QColor(255, 0, 0), 2);
			drawLine(graphicsSceneRGB, h, i, greenHistogram[h], greenHistogram[i], scaleRGB, QColor(0, 255, 0), 1);
			drawLine(graphicsSceneRGB, h, i, blueHistogram[h], blueHistogram[i], scaleRGB, QColor(0, 0, 255), 0);
		}
		else
		{
			drawBar(graphicsSceneIntensity, i, scaleI * intensityHistogram[i], Qt::white, 0);
			
			double r, g, b;
			r = scaleRGB * redHistogram[i];
			g = scaleRGB * greenHistogram[i];
			b = scaleRGB * blueHistogram[i];
			
			drawBar(graphicsSceneRGB, i, r, QColor(255, 0, 0), 0);
			drawBar(graphicsSceneRGB, i, g, QColor(0, 255, 0), 1);
			drawBar(graphicsSceneRGB, i, b, QColor(0, 0, 255), 2);
			drawBar(graphicsSceneRGB, i, qMin(r,g), QColor(255, 255, 0), 3);
			drawBar(graphicsSceneRGB, i, qMin(g,b), QColor(0, 255, 255), 4);
			drawBar(graphicsSceneRGB, i, qMin(b,r), QColor(255, 0, 255), 5);
			drawBar(graphicsSceneRGB, i, qMin(r,qMin(g,b)), Qt::white, 6);
		}
	}

}

void HistogramDialog::clearData()
{
	pixelCount = 0;
	std::fill(intensityHistogram.begin(), intensityHistogram.end(), 0);
	std::fill(redHistogram.begin(), redHistogram.end(), 0);
	std::fill(greenHistogram.begin(), greenHistogram.end(), 0);
	std::fill(blueHistogram.begin(), blueHistogram.end(), 0);
}

void HistogramDialog::processImage(QImage image)
{
	clearData();
	if (!image.isNull())
	{
		if (image.format() == QImage::Format_Indexed8)
		{
			uint64_t * colorHistogram = new uint64_t[256];
			for (int i = 0; i < 256; i++)
			{
				colorHistogram[i] = 0;
			}
			for (int y = 0; y < image.height(); y++)
			{
				const uint8_t * row = reinterpret_cast<const uint8_t *>(image.constScanLine(y)); 
				for (int x = 0; x < image.width(); x++)
				{
					colorHistogram[row[x]] += 1;
				}
			}
			auto colorTable = image.colorTable();
			for (int i = 0; i < qMin(256, colorTable.length()); i++)
			{
				const QRgb pixel = colorTable[i];
				
				intensityHistogram[qBound(0, QColor(pixel).value(), 255)] += colorHistogram[i];
				redHistogram[qRed(pixel)] += colorHistogram[i];
				greenHistogram[qGreen(pixel)] += colorHistogram[i];
				blueHistogram[qBlue(pixel)] += colorHistogram[i];
			}
			
			delete [] colorHistogram;
			
			pixelCount = image.height();
			pixelCount *= image.width();
		}
		else
		{
			if (!((image.format() == QImage::Format_RGB32) || (image.format() == QImage::Format_ARGB32)))
			{
				if (image.hasAlphaChannel())
				{
					image.convertTo(QImage::Format_ARGB32);
				}
				else
				{
					image.convertTo(QImage::Format_RGB32);
				}
			}
			
			int threadCount = Globals::getThreadCount();
			uint64_t * intensityHistogramPartial = new uint64_t[256*threadCount];
			uint64_t * redHistogramPartial = new uint64_t[256*threadCount];
			uint64_t * greenHistogramPartial = new uint64_t[256*threadCount];
			uint64_t * blueHistogramPartial = new uint64_t[256*threadCount];
			for (int i = 0; i < 256*threadCount; i++)
			{
				intensityHistogramPartial[i] = 0;
				redHistogramPartial[i] = 0;
				greenHistogramPartial[i] = 0;
				blueHistogramPartial[i] = 0;
			}
			omp_set_num_threads(threadCount);
			#pragma omp parallel for schedule(dynamic, 1)
			for (int y = 0; y < image.height(); y++)
			{
				int threadNum = omp_get_thread_num();
				assert(threadNum < threadCount);
				uint64_t * intensity = intensityHistogramPartial + 256*threadNum;
				uint64_t * red = redHistogramPartial + 256*threadNum;
				uint64_t * green = greenHistogramPartial + 256*threadNum;
				uint64_t * blue = blueHistogramPartial + 256*threadNum;
				const QRgb * row = reinterpret_cast<const QRgb *>(image.constScanLine(y)); 
				for (int x = 0; x < image.width(); x++)
				{
					const QRgb pixel = row[x];
					red[qRed(pixel)] += 1;
					green[qGreen(pixel)] += 1;
					blue[qBlue(pixel)] += 1;
					intensity[qBound(0, QColor(pixel).value(), 255)] += 1;
				}
			}
			for (int k = 0; k < threadCount; k++)
			{
				for (int i = 0; i < 256; i++)
				{
					intensityHistogram[i] += intensityHistogramPartial[k*256 + i];
					redHistogram[i] += redHistogramPartial[k*256 + i];
					greenHistogram[i] += greenHistogramPartial[k*256 + i];
					blueHistogram[i] += blueHistogramPartial[k*256 + i];
				}
			}
			
			delete [] intensityHistogramPartial;
			delete [] redHistogramPartial;
			delete [] greenHistogramPartial;
			delete [] blueHistogramPartial;
			
			pixelCount = image.height();
			pixelCount *= image.width();
		}
	}
	
	printData(0);
	drawHistogram();
}

