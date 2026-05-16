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


#include "drawdevices.h"
#include "globals.h"
#include "lib/imageOp.h"
#include <omp.h>
#include <cmath>
#include <QDebug>
#include <QPainter>
#include <stack>

// Base class

DrawDevice::DrawDevice(MainWindow * mainWindow, DrawDeviceParameters * parameters, int specialization)
{
	this->mainWindow = mainWindow;
	this->parameters = parameters;
	this->specialization = specialization;
	surface = NULL;
}

void DrawDevice::registerSurface(DisplaySurface * surface)
{
	this->surface = surface;
}

void DrawDevice::disconnectSurface(void)
{
	if (surface)
	{
		abortDraw();
	}
	surface = NULL;
}


////////////////
// Brush

DrawDevicePencil::DrawDevicePencil(MainWindow * mainWindow, DrawDeviceParameters * parameters, int specialization) : DrawDevice(mainWindow, parameters, specialization)
{
	button = 0;
}

void DrawDevicePencil::abortDraw(void)
{
	button = 0;
}

bool DrawDevicePencil::drawLine(QImage & img, QPoint begin, QPoint end)
{
	QPoint dp = end - begin;
	if ((abs(dp.x()) == 0) && (abs(dp.y()) == 0)) return false;
	if ((abs(dp.x()) < 2) && (abs(dp.y()) < 2))
	{
		if (img.rect().contains(end))
		{
			img.setPixelColor(end, color);
		}
		return true;
	}
	
	int x0 = begin.x();
	int x1 = end.x();
	int y0 = begin.y();
	int y1 = end.y();
	int dx = abs(dp.x());
	int sx = (dp.x() > 0) ? 1 : -1;
	int dy = -abs(dp.y());
	int sy = (dp.y() > 0) ? 1 : -1;
	int error = dx + dy;
	QRect rect = img.rect();
	
	while (1)
	{
		if (rect.contains(x0, y0))
		{
			img.setPixelColor(x0, y0, color);
		}
		int e2 = 2 * error;
		if (e2 >= dy)
		{
			if (x0 == x1) break;
			error = error + dy;
			x0 = x0 + sx;
		}
		if (e2 <= dx)
		{
			if (y0 == y1) break;
			error = error + dx;
			y0 = y0 + sy;
		}
	}
	return true;
}

void DrawDevicePencil::mousePressEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(modifiers);
	if ((!button) && ((buttons & Qt::LeftButton) || (buttons & Qt::RightButton)))
	{
		QPoint intPosition = QPoint(position.x(), position.y());
		QImage & img = parameters->display->getImageRef();
		if (!img.rect().contains(intPosition)) return;
		if ((buttons & Qt::LeftButton))
		{
			button = Qt::LeftButton;
			color = parameters->foregroundColor;
		}
		else
		{
			button = Qt::RightButton;
			color = parameters->backgroundColor;
		}
		mainWindow->saveToUndoStack();
		img.setPixelColor(intPosition, color);
		previousPos = intPosition;
		parameters->display->updateInternalImage();
	}
}

void DrawDevicePencil::mouseMoveEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(buttons);
	Q_UNUSED(modifiers);
	if (button)
	{
		QPoint intPosition = QPoint(position.x(), position.y());
		if (drawLine(parameters->display->getImageRef(), previousPos, intPosition))
		{
			parameters->display->updateInternalImage();
		}
		previousPos = intPosition;
	}
}

void DrawDevicePencil::mouseReleaseEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(modifiers);
	if (button && (button & buttons))
	{
		QPoint intPosition = QPoint(position.x(), position.y());
		if (drawLine(parameters->display->getImageRef(), previousPos, intPosition))
		{
			parameters->display->updateInternalImage();
		}
		button = 0;
	}
}



////////////////
// Brush

DrawDeviceBrush::DrawDeviceBrush(MainWindow * mainWindow, DrawDeviceParameters * parameters, int specialization) : DrawDevice(mainWindow, parameters, specialization)
{
	pathItem = NULL;
	button = 0;
}

void DrawDeviceBrush::abortDraw(void)
{
	button = 0;
	if (pathItem)
	{
		surface->removeItem(pathItem);
		delete pathItem;
		pathItem = NULL;
	}
}

void DrawDeviceBrush::mousePressEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(modifiers);
	if ((!button) && ((buttons & Qt::LeftButton) || (buttons & Qt::RightButton)))
	{
		path = QPainterPath(position / Globals::scalingFactor);
		if ((buttons & Qt::LeftButton))
		{
			button = Qt::LeftButton;
			color = parameters->foregroundColor;
		}
		else
		{
			button = Qt::RightButton;
			color = parameters->backgroundColor;
		}
		pathItem = surface->addPath(path, QPen(QBrush(color, Qt::SolidPattern), parameters->width / Globals::scalingFactor, parameters->lineStyle, parameters->lineEnd, parameters->lineJoin));
		pathItem->setZValue(999);
	}
}

void DrawDeviceBrush::mouseMoveEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(buttons);
	Q_UNUSED(modifiers);
	if (pathItem)
	{
		path.lineTo(position / Globals::scalingFactor);
		pathItem->setPath(path);
	}
}

void DrawDeviceBrush::mouseReleaseEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(modifiers);
	if (pathItem && button && (button & buttons))
	{
		path.lineTo(position / Globals::scalingFactor);
		surface->removeItem(pathItem);
		delete pathItem;
		pathItem = NULL;
		mainWindow->saveToUndoStack();
		QImage & img = parameters->display->getImageRef();
		img.setDevicePixelRatio(Globals::scalingFactor);
		QPainter painter(&(img));
		painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
		painter.setPen(QPen(QBrush(color, Qt::SolidPattern), parameters->width / Globals::scalingFactor, parameters->lineStyle, parameters->lineEnd, parameters->lineJoin));
		painter.setRenderHint(QPainter::Antialiasing, parameters->antialiasing);
		painter.drawPath(path);
		painter.end();
		img.setDevicePixelRatio(1.0);
		parameters->display->updateInternalImage();
		button = 0;
	}
}



////////////////
// Smudge

DrawDeviceSmudge::DrawDeviceSmudge(MainWindow * mainWindow, DrawDeviceParameters * parameters, int specialization) : DrawDevice(mainWindow, parameters, specialization)
{
	sourceCircle = NULL;
	sourceCircleDecor = NULL;
	deltaXmap = NULL;
	deltaYmap = NULL;
	deltaMapSize = -1;
	button = 0;
}

void DrawDeviceSmudge::abortDraw(void)
{
	button = 0;
	if (sourceCircle)
	{
		surface->removeItem(sourceCircle);
		delete sourceCircle;
		sourceCircle = NULL;
		surface->removeItem(sourceCircleDecor);
		delete sourceCircleDecor;
		sourceCircleDecor = NULL;
	}
	if (deltaXmap)
	{
		delete [] deltaXmap;
		deltaXmap = NULL;
	}
	if (deltaYmap)
	{
		delete [] deltaYmap;
		deltaYmap = NULL;
	}
	deltaMapSize = -1;
	sourceImg = QImage();
	maskImg = QImage();
}

void DrawDeviceSmudge::drawTargetCircle(QPoint position)
{
	if (!parameters->display->getImageRef().rect().contains(position))
	{
		if (sourceCircle)
		{
			surface->removeItem(sourceCircle);
			delete sourceCircle;
			sourceCircle = NULL;
			surface->removeItem(sourceCircleDecor);
			delete sourceCircleDecor;
			sourceCircleDecor = NULL;
		}
		return;
	}
	if (sourceCircle == NULL)
	{
		sourceCircle = surface->addEllipse(QRectF(), QPen(QBrush(QColor(Qt::black), Qt::SolidPattern), 2 / Globals::scalingFactor), QBrush());
		sourceCircleDecor = surface->addEllipse(QRectF(), QPen(QBrush(QColor(Qt::white), Qt::Dense2Pattern), 2 / Globals::scalingFactor), QBrush());
		sourceCircle->setZValue(998);
		sourceCircleDecor->setZValue(999);
	}
	QRectF circle = QRectF(QPointF(), QSizeF(parameters->size*2 + 1, parameters->size*2 + 1) / Globals::scalingFactor);
	circle.moveCenter(QPointF(position) / Globals::scalingFactor);
	sourceCircle->setRect(circle);
	sourceCircleDecor->setRect(circle);
}

void DrawDeviceSmudge::copyPatch(QImage & img, QPoint position)
{
	QPoint deltaPos = previousPos - position;
	if (warpMode && deltaPos.isNull()) return;
	
	patchRect.moveCenter(position);
	
	QImage srcPatch = sourceImg.copy(patchRect).convertToFormat(QImage::Format_ARGB32);
	QImage srcMaskPatch = maskImg.copy(patchRect).convertToFormat(QImage::Format_ARGB32);
	QImage dstPatch = img.copy(patchRect).convertToFormat(QImage::Format_ARGB32);
	
	if (warpMode)
	{
		int centerX = patchRect.width() / 2;
		int centerY = patchRect.height() / 2;
		
		int mX = patchRect.x();
		int mY = patchRect.y();
		int mW = sourceImg.width();
		int mH = sourceImg.height();
		int w = patchRect.width();
		int h = patchRect.height();
		
		for (int y = 0; y < h; y++)
		{
			double rY = abs(y - centerY);
			rY /= centerY;
			for (int x = 0; x < w; x++)
			{
				double rX = abs(x - centerX);
				rX /= centerX;
				double l = std::sqrt(rX*rX + rY*rY);
				double r = 0.0;
				l *= centerX;
				l /= (centerX - 1);
				if (l < 1.0)
				{
					r = 1.0 - l;
				}
				r *= 0.95;
				
				double dX = r * qBound(-1, deltaPos.x(), 1);
				double dY = r * qBound(-1, deltaPos.y(), 1);
				
				int imX = mX + x;
				int imY = mY + y;
				if (imX < 0) continue;
				if (imY < 0) continue;
				if (imX >= mW) continue;
				if (imY >= mH) continue;
				
				deltaXmap[mW*imY + imX] += dX;
				deltaYmap[mW*imY + imX] += dY;
			}
		}
		for (int y = 0; y < patchRect.height(); y++)
		{
			for (int x = 0; x < patchRect.width(); x++)
			{
				int imX = mX + x;
				int imY = mY + y;
				if (imX < 0) continue;
				if (imY < 0) continue;
				if (imX >= mW) continue;
				if (imY >= mH) continue;
				
				double dX = deltaXmap[mW*imY + imX];
				double dY = deltaYmap[mW*imY + imX];
				
				double fX = mX + x + dX;
				double fY = mY + y + dY;
				if (fX < 0.0) fX = 0.0;
				if (fX > mW) fX = mW;
				if (fY < 0.0) fY = 0.0;
				if (fY > mH) fY = mH;
				
				
				int iX = fX;
				int iY = fY;
				int ifX = (fX - iX) * 1024;
				int ifY = (fY - iY) * 1024;
				int iXn = qMin(iX + 1, mW);
				int iYn = qMin(iY + 1, mH);
				QRgb color00 = sourceImg.pixelColor(iX, iY).rgba();
				QRgb color01 = sourceImg.pixelColor(iXn, iY).rgba();
				QRgb color10 = sourceImg.pixelColor(iX, iYn).rgba();
				QRgb color11 = sourceImg.pixelColor(iXn, iYn).rgba();
				int alpha = (1024 - ifY) * ((1024 - ifX) * qAlpha(color00) + ifX * qAlpha(color01)) + ifY * ((1024 - ifX) * qAlpha(color10) + ifX * qAlpha(color11));
				int red   = (1024 - ifY) * ((1024 - ifX) * qRed(color00)   + ifX * qRed(color01))   + ifY * ((1024 - ifX) * qRed(color10)   + ifX * qRed(color11));
				int green = (1024 - ifY) * ((1024 - ifX) * qGreen(color00) + ifX * qGreen(color01)) + ifY * ((1024 - ifX) * qGreen(color10) + ifX * qGreen(color11));
				int blue  = (1024 - ifY) * ((1024 - ifX) * qBlue(color00)  + ifX * qBlue(color01))  + ifY * ((1024 - ifX) * qBlue(color10)  + ifX * qBlue(color11));
				alpha /= (1024 * 1024);
				red   /= (1024 * 1024);
				green /= (1024 * 1024);
				blue  /= (1024 * 1024);
				QColor interpolatedColor = QColor::fromRgb(qRgba(red, green, blue, alpha));
				srcPatch.setPixelColor(x, y, interpolatedColor);
			}
		}
	} else
	if (warpBlurMode)
	{
		int centerX = patchRect.width() / 2;
		int centerY = patchRect.height() / 2;
		int w = patchRect.width();
		int h = patchRect.height();
		
		for (int y = 0; y < h; y++)
		{
			double rY = abs(y - centerY);
			rY /= centerY;
			for (int x = 0; x < w; x++)
			{
				double rX = abs(x - centerX);
				rX /= centerX;
				double l = std::sqrt(rX*rX + rY*rY);
				double r = 0.0;
				l *= centerX;
				l /= (centerX - 1);
				if (l < 1.0)
				{
					r = 1.0 - l;
				}
				r *= 0.95;
				
				double fX = x + r * qBound(-1, deltaPos.x(), 1);
				double fY = y + r * qBound(-1, deltaPos.y(), 1);
				if (fX < 0.0) fX = 0.0;
				if (fX > w) fX = w;
				if (fY < 0.0) fY = 0.0;
				if (fY > h) fY = h;
				
				int iX = fX;
				int iY = fY;
				int ifX = (fX - iX) * 1024;
				int ifY = (fY - iY) * 1024;
				int iXn = qMin(iX + 1, w);
				int iYn = qMin(iY + 1, h);
				QRgb color00 = dstPatch.pixelColor(iX, iY).rgba();
				QRgb color01 = dstPatch.pixelColor(iXn, iY).rgba();
				QRgb color10 = dstPatch.pixelColor(iX, iYn).rgba();
				QRgb color11 = dstPatch.pixelColor(iXn, iYn).rgba();
				int alpha = (1024 - ifY) * ((1024 - ifX) * qAlpha(color00) + ifX * qAlpha(color01)) + ifY * ((1024 - ifX) * qAlpha(color10) + ifX * qAlpha(color11));
				int red   = (1024 - ifY) * ((1024 - ifX) * qRed(color00)   + ifX * qRed(color01))   + ifY * ((1024 - ifX) * qRed(color10)   + ifX * qRed(color11));
				int green = (1024 - ifY) * ((1024 - ifX) * qGreen(color00) + ifX * qGreen(color01)) + ifY * ((1024 - ifX) * qGreen(color10) + ifX * qGreen(color11));
				int blue  = (1024 - ifY) * ((1024 - ifX) * qBlue(color00)  + ifX * qBlue(color01))  + ifY * ((1024 - ifX) * qBlue(color10)  + ifX * qBlue(color11));
				alpha /= (1024 * 1024);
				red   /= (1024 * 1024);
				green /= (1024 * 1024);
				blue  /= (1024 * 1024);
				QColor interpolatedColor = QColor::fromRgb(qRgba(red, green, blue, alpha));
				srcPatch.setPixelColor(x, y, interpolatedColor);
			}
		}
	}
	else
	{
		srcPatch = ImageOp::Blur(srcPatch, (parameters->radius + 2)/2.0);
		srcPatch.setAlphaChannel(patchMask);
	}
	srcPatch.setAlphaChannel(srcMaskPatch);
	
	QPainter painterPatch(&dstPatch);
	painterPatch.setCompositionMode(QPainter::CompositionMode_SourceOver);
	painterPatch.drawImage(QPoint(), srcPatch);
	painterPatch.end();
	
	dstPatch = dstPatch.convertToFormat(img.format());
	
	QPainter painter(&img);
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	painter.drawImage(patchRect, dstPatch);
	painter.end();
}

void DrawDeviceSmudge::mousePressEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	QPoint intPosition = QPoint(position.x(), position.y());
	drawTargetCircle(intPosition);
	if (buttons & Qt::LeftButton)
	{
		button = Qt::LeftButton;
		previousPos = intPosition;
		warpMode = (modifiers & Qt::ControlModifier);
		warpBlurMode = (modifiers & Qt::ShiftModifier) && !warpMode;
		
		if (warpMode)
		{
			int size = parameters->display->getImageRef().width() * parameters->display->getImageRef().height();
			if ((deltaMapSize != size) || (deltaXmap == NULL) || (deltaYmap == NULL))
			{
				if (deltaXmap)
				{
					delete [] deltaXmap;
				}
				if (deltaYmap)
				{
					delete [] deltaYmap;
				}
				deltaXmap = new float[size];
				deltaYmap = new float[size];
			}
			deltaMapSize = size;
			memset(deltaXmap, 0, sizeof(float) * size);
			memset(deltaYmap, 0, sizeof(float) * size);
		}
		
		mainWindow->saveToUndoStack();
		sourceImg = parameters->display->getImage();
		maskImg = QImage(sourceImg.size(), QImage::Format_MonoLSB);
		maskImg.fill(0xFF);
		patchRect = QRect(QPoint(), QSize(parameters->size*2 + 1, parameters->size*2 + 1));
		patchMask = QImage(patchRect.size(), QImage::Format_Grayscale8);
		int r = parameters->size;
		for (int y = -r; y <= r; y++)
		{
			uint8_t * row = reinterpret_cast<uint8_t *>(patchMask.scanLine(y + r));
			for (int x = -r; x <= r; x++)
			{
				double l = std::sqrt(y*y + x*x);
				l /= r;
				if (l > 1.0)
				{
					row[x + r] = 0;
				}
				else
				{
					l = 1.0 - l;
					l *= 1.5;
					l = l*l*l;
					l = qBound(0.0, l, 1.0);
					int level = 64 * l;
					row[x + r] = qBound(0, level, 255);
				}
			}
		}
		copyPatch(parameters->display->getImageRef(), intPosition);
		parameters->display->updateInternalImage();
	}
}

void DrawDeviceSmudge::mouseMoveEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(buttons);
	Q_UNUSED(modifiers);
	QPoint intPosition = QPoint(position.x(), position.y());
	drawTargetCircle(intPosition);
	if (button)
	{
		copyPatch(parameters->display->getImageRef(), intPosition);
		parameters->display->updateInternalImage();
		previousPos = intPosition;
	}
}

void DrawDeviceSmudge::mouseReleaseEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(modifiers);
	if (button && (button & buttons))
	{
		QPoint intPosition = QPoint(position.x(), position.y());
		drawTargetCircle(intPosition);
		sourceImg = QImage();
		maskImg = QImage();
	}
}


////////////////
// Clone

DrawDeviceClone::DrawDeviceClone(MainWindow * mainWindow, DrawDeviceParameters * parameters, int specialization) : DrawDevice(mainWindow, parameters, specialization)
{
	state = 0;
	sourceCircle = NULL;
	sourceCircleDecor = NULL;
}

void DrawDeviceClone::abortDraw(void)
{
	state = 0;
	if (sourceCircle)
	{
		surface->removeItem(sourceCircle);
		delete sourceCircle;
		sourceCircle = NULL;
		surface->removeItem(sourceCircleDecor);
		delete sourceCircleDecor;
		sourceCircleDecor = NULL;
	}
	sourceImg = QImage();
	maskImg = QImage();
}

void DrawDeviceClone::drawSourceCircle(void)
{
	if (sourceCircle == NULL)
	{
		sourceCircle = surface->addEllipse(QRectF(), QPen(QBrush(QColor(Qt::black), Qt::SolidPattern), 2 / Globals::scalingFactor), QBrush());
		sourceCircleDecor = surface->addEllipse(QRectF(), QPen(QBrush(QColor(Qt::white), Qt::Dense2Pattern), 2 / Globals::scalingFactor), QBrush());
		sourceCircle->setZValue(998);
		sourceCircleDecor->setZValue(999);
	}
	QRectF circle = QRectF(QPointF(), QSizeF(parameters->size*2 + 1, parameters->size*2 + 1) / Globals::scalingFactor);
	circle.moveCenter(QPointF(sourcePos) / Globals::scalingFactor);
	sourceCircle->setRect(circle);
	sourceCircleDecor->setRect(circle);
}

void DrawDeviceClone::copyPatch(QImage & img, QPoint src, QPoint dst)
{
	patchRect.moveCenter(src);
	
	QImage srcPatch = sourceImg.copy(patchRect).convertToFormat(QImage::Format_ARGB32);
	QImage srcMaskPatch = maskImg.copy(patchRect).convertToFormat(QImage::Format_ARGB32);
	patchRect.moveCenter(dst);
	QImage dstPatch = img.copy(patchRect).convertToFormat(QImage::Format_ARGB32);
	
	srcPatch.setAlphaChannel(patchMask);
	srcPatch.setAlphaChannel(srcMaskPatch);
	
	QPainter painterPatch(&dstPatch);
	painterPatch.setCompositionMode(QPainter::CompositionMode_SourceOver);
	painterPatch.drawImage(QPoint(), srcPatch);
	painterPatch.end();
	
	dstPatch = dstPatch.convertToFormat(img.format());
	
	QPainter painter(&img);
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	patchRect.moveCenter(dst);
	painter.drawImage(patchRect, dstPatch);
	painter.end();
}

void DrawDeviceClone::mousePressEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	if (buttons & Qt::LeftButton)
	{
		QPoint intPosition = QPoint(position.x(), position.y());
		if (modifiers == Qt::ControlModifier)
		{
			state = 1;
			sourcePos = intPosition;
			drawSourceCircle();
		}
		else
		{
			if ((state == 2) || (state == 3))
			{
				deltaPos = intPosition - sourcePos;
				state = 4;
				mainWindow->saveToUndoStack();
				sourceImg = parameters->display->getImage();
				maskImg = QImage(sourceImg.size(), QImage::Format_MonoLSB);
				maskImg.fill(0xFF);
				patchRect = QRect(QPoint(), QSize(parameters->size*2 + 1, parameters->size*2 + 1));
				patchMask = QImage(patchRect.size(), QImage::Format_Grayscale8);
				int r = parameters->size;
				for (int y = -r; y <= r; y++)
				{
					uint8_t * row = reinterpret_cast<uint8_t *>(patchMask.scanLine(y + r));
					for (int x = -r; x <= r; x++)
					{
						double l = std::sqrt(y*y + x*x);
						l /= r;
						if (l > 1.0)
						{
							row[x + r] = 0;
						}
						else
						{
							l = 1.0 - l;
							l *= 1.5;
							l = l*l*l;
							int level = 255 * l;
							row[x + r] = qBound(0, level, 255);
						}
					}
				}
				copyPatch(parameters->display->getImageRef(), sourcePos, intPosition);
				parameters->display->updateInternalImage();
			}
		}
	}
}

void DrawDeviceClone::mouseMoveEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(buttons);
	Q_UNUSED(modifiers);
	QPoint intPosition = QPoint(position.x(), position.y());
	if (state == 1)
	{
		sourcePos = intPosition;
		drawSourceCircle();
	} else
	if (state == 3)
	{
		sourcePos = intPosition - deltaPos;
		drawSourceCircle();
	} else
	if (state == 4)
	{
		sourcePos = intPosition - deltaPos;
		drawSourceCircle();
		copyPatch(parameters->display->getImageRef(), sourcePos, intPosition);
		parameters->display->updateInternalImage();
	}
}

void DrawDeviceClone::mouseReleaseEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(modifiers);
	if (buttons & Qt::LeftButton)
	{
		QPoint intPosition = QPoint(position.x(), position.y());
		if (state == 1)
		{
			sourcePos = intPosition;
			drawSourceCircle();
			state = 2;
			
		} else
		if (state == 4)
		{
			state = 3;
			sourceImg = QImage();
			maskImg = QImage();
		}
	}
}

////////////////
// Line

DrawDeviceLine::DrawDeviceLine(MainWindow * mainWindow, DrawDeviceParameters * parameters, int specialization) : DrawDevice(mainWindow, parameters, specialization)
{
	lineItem = NULL;
	button = 0;
}

void DrawDeviceLine::abortDraw(void)
{
	button = 0;
	if (lineItem)
	{
		surface->removeItem(lineItem);
		delete lineItem;
		lineItem = NULL;
	}
}
#include <QGraphicsBlurEffect>
void DrawDeviceLine::mousePressEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(modifiers);
	if ((!button) && ((buttons & Qt::LeftButton) || (buttons & Qt::RightButton)))
	{
		line = QLineF(position / Globals::scalingFactor, position / Globals::scalingFactor);
		if ((buttons & Qt::LeftButton))
		{
			button = Qt::LeftButton;
			color = parameters->foregroundColor;
		}
		else
		{
			button = Qt::RightButton;
			color = parameters->backgroundColor;
		}
		lineItem = surface->addLine(line, QPen(QBrush(color, Qt::SolidPattern), parameters->width / Globals::scalingFactor, parameters->lineStyle, parameters->lineEnd, parameters->lineJoin));
		lineItem->setZValue(999);
	}
}

void DrawDeviceLine::mouseMoveEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(buttons);
	if (lineItem)
	{
		if (modifiers == Qt::NoModifier)
		{
			line.setP2(position / Globals::scalingFactor);
		} else
		if (modifiers & Qt::ControlModifier)
		{
			line.setP1(line.p1() - line.p2() + position / Globals::scalingFactor);
			line.setP2(position / Globals::scalingFactor);
		} else
		if (modifiers == Qt::ShiftModifier)
		{
			line.setP2(position / Globals::scalingFactor);
			int a = line.angle();
			a /= 15;
			a *= 15;
			line.setAngle(a);
		}
		lineItem->setLine(line);
	}
}

void DrawDeviceLine::mouseReleaseEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	if (lineItem && button && (button & buttons))
	{
		if (modifiers == Qt::NoModifier)
		{
			line.setP2(position / Globals::scalingFactor);
		} else
		if (modifiers & Qt::ControlModifier)
		{
			line.setP1(line.p1() - line.p2() + position / Globals::scalingFactor);
			line.setP2(position / Globals::scalingFactor);
		} else
		if (modifiers == Qt::ShiftModifier)
		{
			line.setP2(position / Globals::scalingFactor);
			int a = line.angle();
			a /= 15;
			a *= 15;
			line.setAngle(a);
		}
		surface->removeItem(lineItem);
		delete lineItem;
		lineItem = NULL;
		mainWindow->saveToUndoStack();
		QImage & img = parameters->display->getImageRef();
		QPainter painter(&(img));
		painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
		painter.setPen(QPen(QBrush(color, Qt::SolidPattern), parameters->width, parameters->lineStyle, parameters->lineEnd, parameters->lineJoin));
		painter.setRenderHint(QPainter::Antialiasing, parameters->antialiasing);
		painter.drawLine(QLineF(line.p1() * Globals::scalingFactor, line.p2() * Globals::scalingFactor));
		painter.end();
		parameters->display->updateInternalImage();
		button = 0;
	}
}

////////////////
// Shapes

DrawDeviceShapes::DrawDeviceShapes(MainWindow * mainWindow, DrawDeviceParameters * parameters, int specialization) : DrawDevice(mainWindow, parameters, specialization)
{
	rectItem = NULL;
	roundedRectItem = NULL;
	ellipseItem = NULL;
	button = 0;
}

void DrawDeviceShapes::abortDraw(void)
{
	button = 0;
	if (rectItem)
	{
		surface->removeItem(rectItem);
		delete rectItem;
		rectItem = NULL;
	}
	if (roundedRectItem)
	{
		surface->removeItem(roundedRectItem);
		delete roundedRectItem;
		roundedRectItem = NULL;
	}
	if (ellipseItem)
	{
		surface->removeItem(ellipseItem);
		delete ellipseItem;
		ellipseItem = NULL;
	}
}

void DrawDeviceShapes::mousePressEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(modifiers);
	if ((!button) && ((buttons & Qt::LeftButton) || (buttons & Qt::RightButton)))
	{
		rect = QRectF(position / Globals::scalingFactor, QSizeF());
		if ((buttons & Qt::LeftButton))
		{
			button = Qt::LeftButton;
			color = parameters->foregroundColor;
			fillColor = parameters->backgroundColor;
		}
		else
		{
			button = Qt::RightButton;
			color = parameters->backgroundColor;
			fillColor = parameters->foregroundColor;
		}
		QPen pen = QPen(QBrush(color, (parameters->polygonFillOutline < 2) ? Qt::SolidPattern : Qt::NoBrush), parameters->width / Globals::scalingFactor, parameters->lineStyle, parameters->lineEnd, parameters->lineJoin);
		QBrush brush = QBrush(fillColor, (parameters->polygonFillOutline > 0) ? Qt::SolidPattern : Qt::NoBrush);
		switch (specialization)
		{
			default:
			case Specialization::Rectangle:
				rectItem = surface->addRect(rect, pen, brush);
				rectItem->setZValue(999);
				break;
			case Specialization::RoundedRectangle:
				{
					QPainterPath roundedRectPath;
					double radius = parameters->radius / Globals::scalingFactor;
					roundedRectPath.addRoundedRect(rect, radius, radius);
					roundedRectItem = surface->addPath(roundedRectPath, pen, brush);
					roundedRectItem->setZValue(999);
				}
				break;
			case Specialization::Ellipse:
				ellipseItem = surface->addEllipse(rect, pen, brush);
				ellipseItem->setZValue(999);
				break;
		}
	}
}

void DrawDeviceShapes::mouseMoveEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(buttons);
	if (rectItem || ellipseItem || roundedRectItem)
	{
		if (modifiers == Qt::NoModifier)
		{
			rect.setBottomRight(position / Globals::scalingFactor);
		} else
		if (modifiers & Qt::ControlModifier)
		{
			rect.moveBottomRight(position / Globals::scalingFactor);
		} else
		if (modifiers == Qt::ShiftModifier)
		{
			rect.setBottomRight(position / Globals::scalingFactor);
			if (std::fabs(rect.width()) < std::fabs(rect.height()))
			{
				rect.setHeight(std::copysign(rect.width(), rect.height()));
			} else
			if (std::fabs(rect.width()) > std::fabs(rect.height()))
			{
				rect.setWidth(std::copysign(rect.height(), rect.width()));
			}
		}
		if (rectItem)
		{
			rectItem->setRect(rect.normalized());
		}
		if (roundedRectItem)
		{
			QPainterPath roundedRectPath;
			double radius = parameters->radius / Globals::scalingFactor;
			roundedRectPath.addRoundedRect(rect.normalized(), radius, radius);
			roundedRectItem->setPath(roundedRectPath);
		}
		if (ellipseItem)
		{
			ellipseItem->setRect(rect.normalized());
		}
	}
}

void DrawDeviceShapes::mouseReleaseEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	if ((rectItem || ellipseItem || roundedRectItem) && button && (button & buttons))
	{
		if (modifiers == Qt::NoModifier)
		{
			rect.setBottomRight(position / Globals::scalingFactor);
		} else
		if (modifiers & Qt::ControlModifier)
		{
			rect.moveBottomRight(position / Globals::scalingFactor);
		} else
		if (modifiers == Qt::ShiftModifier)
		{
			rect.setBottomRight(position / Globals::scalingFactor);
			if (std::fabs(rect.width()) < std::fabs(rect.height()))
			{
				rect.setHeight(std::copysign(rect.width(), rect.height()));
			} else
			if (std::fabs(rect.width()) > std::fabs(rect.height()))
			{
				rect.setWidth(std::copysign(rect.height(), rect.width()));
			}
		}
		if (rectItem)
		{
			surface->removeItem(rectItem);
			delete rectItem;
			rectItem = NULL;
		}
		if (roundedRectItem)
		{
			surface->removeItem(roundedRectItem);
			delete roundedRectItem;
			roundedRectItem = NULL;
		}
		if (ellipseItem)
		{
			surface->removeItem(ellipseItem);
			delete ellipseItem;
			ellipseItem = NULL;
		}
		mainWindow->saveToUndoStack();
		QImage & img = parameters->display->getImageRef();
		QPainter painter(&(img));
		painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
		painter.setPen(QPen(QBrush(color, (parameters->polygonFillOutline < 2) ? Qt::SolidPattern : Qt::NoBrush), parameters->width, parameters->lineStyle, parameters->lineEnd, parameters->lineJoin));
		painter.setBrush(QBrush(fillColor, (parameters->polygonFillOutline > 0) ? Qt::SolidPattern : Qt::NoBrush));
		painter.setRenderHint(QPainter::Antialiasing, parameters->antialiasing);
		switch (specialization)
		{
			default:
			case Specialization::Rectangle:
				painter.drawRect(QRectF(rect.normalized().topLeft() * Globals::scalingFactor, rect.normalized().size() * Globals::scalingFactor));
				break;
			case Specialization::RoundedRectangle:
				painter.drawRoundedRect(QRectF(rect.normalized().topLeft() * Globals::scalingFactor, rect.normalized().size() * Globals::scalingFactor), parameters->radius, parameters->radius);
				break;
			case Specialization::Ellipse:
				painter.drawEllipse(QRectF(rect.normalized().topLeft() * Globals::scalingFactor, rect.normalized().size() * Globals::scalingFactor));
				break;
		}
		painter.end();
		parameters->display->updateInternalImage();
		button = 0;
	}
}


////////////////
// Color picker

DrawDeviceColorPicker::DrawDeviceColorPicker(MainWindow * mainWindow, DrawDeviceParameters * parameters, int specialization) : DrawDevice(mainWindow, parameters, specialization)
{
	button = 0;
}

void DrawDeviceColorPicker::abortDraw(void)
{
	button = 0;
}

void DrawDeviceColorPicker::mousePressEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(modifiers);
	if ((!button) && ((buttons & Qt::LeftButton) || (buttons & Qt::RightButton)))
	{
		QPoint intPosition = QPoint(position.x(), position.y());
		QImage & img = parameters->display->getImageRef();
		if ((buttons & Qt::LeftButton))
		{
			button = Qt::LeftButton;
		}
		else
		{
			button = Qt::RightButton;
		}
		if (button == Qt::LeftButton)
		{
			mainWindow->setDrawForegroundColor(img.pixelColor(intPosition));
		} else
		if (button == Qt::RightButton)
		{
			mainWindow->setDrawBackgroundColor(img.pixelColor(intPosition));
		}
	}
}

void DrawDeviceColorPicker::mouseMoveEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(buttons);
	Q_UNUSED(modifiers);
	if (button)
	{
		QPoint intPosition = QPoint(position.x(), position.y());
		QImage & img = parameters->display->getImageRef();
		if (button == Qt::LeftButton)
		{
			mainWindow->setDrawForegroundColor(img.pixelColor(intPosition));
		} else
		if (button == Qt::RightButton)
		{
			mainWindow->setDrawBackgroundColor(img.pixelColor(intPosition));
		}
	}
}

void DrawDeviceColorPicker::mouseReleaseEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(modifiers);
	if (button && (button & buttons))
	{
		QPoint intPosition = QPoint(position.x(), position.y());
		QImage & img = parameters->display->getImageRef();
		if (button == Qt::LeftButton)
		{
			mainWindow->setDrawForegroundColor(img.pixelColor(intPosition));
		} else
		if (button == Qt::RightButton)
		{
			mainWindow->setDrawBackgroundColor(img.pixelColor(intPosition));
		}
		button = 0;
	}
}

////////////////
// Text

DrawDeviceText::DrawDeviceText(MainWindow * mainWindow, DrawDeviceParameters * parameters, int specialization) : DrawDevice(mainWindow, parameters, specialization)
{
	textItem = NULL;
	button = 0;
}

void DrawDeviceText::abortDraw(void)
{
	button = 0;
	if (textItem)
	{
		surface->removeItem(textItem);
		delete textItem;
		textItem = NULL;
	}
}

void DrawDeviceText::mousePressEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(modifiers);
	if ((!button) && ((buttons & Qt::LeftButton) || (buttons & Qt::RightButton)))
	{
		if ((buttons & Qt::LeftButton))
		{
			button = Qt::LeftButton;
			color = parameters->foregroundColor;
		}
		else
		{
			button = Qt::RightButton;
			color = parameters->backgroundColor;
		}
		QFont font = parameters->font;
		font.setBold(parameters->boldFont);
		font.setItalic(parameters->italicFont);
		font.setPixelSize(parameters->fontSize / Globals::scalingFactor);
		if (parameters->text.isEmpty())
		{
			textItem = surface->addText("Abc", font);
		}
		else
		{
			textItem = surface->addText(parameters->text, font);
		}
		textItem->setPos(position / Globals::scalingFactor);
		textItem->setDefaultTextColor(color);
		textItem->setZValue(999);
		textItem->document()->setDocumentMargin(0); 
	}
}

void DrawDeviceText::mouseMoveEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(buttons);
	Q_UNUSED(modifiers);
	if (textItem)
	{
		textItem->setPos(position / Globals::scalingFactor);
	}
}

void DrawDeviceText::mouseReleaseEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(modifiers);
	if (textItem && (button && (button & buttons)))
	{
		surface->removeItem(textItem);
		delete textItem;
		textItem = NULL;
		mainWindow->saveToUndoStack();
		QImage & img = parameters->display->getImageRef();
		img.setDevicePixelRatio(Globals::scalingFactor);
		QPainter painter(&(img));
		painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
		QFont font = parameters->font;
		font.setBold(parameters->boldFont);
		font.setItalic(parameters->italicFont);
		font.setPixelSize(parameters->fontSize / Globals::scalingFactor);
		painter.setFont(font);
		painter.setPen(QPen(color));
		painter.setRenderHint(QPainter::Antialiasing, parameters->antialiasing);
		painter.drawText(QRectF(position / Globals::scalingFactor, QSizeF(img.size())), Qt::AlignLeft | Qt::AlignTop, parameters->text);
		painter.end();
		img.setDevicePixelRatio(1.0);
		parameters->display->updateInternalImage();
		button = 0;
	}
}


////////////////
// Fill

DrawDeviceFill::DrawDeviceFill(MainWindow * mainWindow, DrawDeviceParameters * parameters, int specialization) : DrawDevice(mainWindow, parameters, specialization)
{
	
}

void DrawDeviceFill::abortDraw(void)
{
	
}

void DrawDeviceFill::fill(QImage & img, QPoint source, int tolerance)
{
	volatile uchar * tmp = img.bits(); Q_UNUSED(tmp);	// break implicit data sharing
	QColor targetColor = img.pixelColor(source);
	QRgb targetRgb, fillRgb;
	int h = img.height();
	int w = img.width();
	bool hasAlpha = img.hasAlphaChannel();
	if (hasAlpha)
	{
		targetRgb = targetColor.rgba();
		fillRgb = fillColor.rgba();
	}
	else
	{
		targetRgb = targetColor.rgb();
		fillRgb = fillColor.rgb();
	}

	
	int lineLen = (w + 31) / 32;
	uint32_t * colorMap = new uint32_t[lineLen * h];
	uint32_t * fillMap = new uint32_t[lineLen * h];
	memset(fillMap, 0, sizeof(uint32_t) * lineLen * h);
	
	omp_set_num_threads(Globals::getThreadCount());
	
	if ((img.format() == QImage::Format_RGB32) || (img.format() == QImage::Format_ARGB32))
	{
		#pragma omp parallel for schedule(static, 1)
		for (int y = 0; y < h; y++)
		{
			const QRgb * row = reinterpret_cast<const QRgb *>(img.constScanLine(y));
			uint32_t * mapRow = colorMap + y*lineLen;
			uint32_t bits = 0;
			for (int x = 0; x < w; x++)
			{
				QRgb srcRgb = row[x];
				uint32_t withinTolerance = 1;
				do {
					if (srcRgb == targetRgb) break;
					withinTolerance = 0;
					if (hasAlpha) if (abs(qAlpha(srcRgb) - qAlpha(targetRgb)) > tolerance) break;
					if (abs(qRed(srcRgb) - qRed(targetRgb)) > tolerance) break;
					if (abs(qGreen(srcRgb) - qGreen(targetRgb)) > tolerance) break;
					if (abs(qBlue(srcRgb) - qBlue(targetRgb)) > tolerance) break;
					withinTolerance = 1;
				} while(0);
				
				bits |= (withinTolerance << (x % 32));
				
				if ((x > 0) && ((x % 32) == 31))
				{
					mapRow[x / 32] = bits;
					bits = 0;
				}
			}
			if (((w - 1) % 32) != 31)
			{
				mapRow[(w - 1) / 32] = bits;
			}
		}
	}
	else
	{
		#pragma omp parallel for schedule(static, 1)
		for (int y = 0; y < h; y++)
		{
			uint32_t * mapRow = colorMap + y*lineLen;
			uint32_t bits = 0;
			for (int x = 0; x < w; x++)
			{
				QColor pixelColor = img.pixelColor(x, y);
				uint32_t withinTolerance = 1;
				do {
					if (pixelColor == targetColor) break;
					withinTolerance = 0;
					QRgb srcRgb;
					if (hasAlpha)
					{
						srcRgb = pixelColor.rgba();
						if (abs(qAlpha(srcRgb) - qAlpha(targetRgb)) > tolerance) break;
					}
					else
					{
						srcRgb = pixelColor.rgb();
					}
					if (abs(qRed(srcRgb) - qRed(targetRgb)) > tolerance) break;
					if (abs(qGreen(srcRgb) - qGreen(targetRgb)) > tolerance) break;
					if (abs(qBlue(srcRgb) - qBlue(targetRgb)) > tolerance) break;
					withinTolerance = 1;
				} while(0);
				
				bits |= (withinTolerance << (x % 32));
				
				if ((x > 0) && ((x % 32) == 31))
				{
					mapRow[x / 32] = bits;
					bits = 0;
				}
			}
			if (((w - 1) % 32) != 31)
			{
				mapRow[(w - 1) / 32] = bits;
			}
		}
	}
	
	std::stack<Span> s;
	
	int x = source.x();
	int y = source.y();
	s.push({x, x, y, 1});
	s.push({x, x, y - 1, -1});
	
	while (!s.empty())
	{
		Span n = s.top();
		s.pop();
		
		int x1 = n.x1;
		int x2 = n.x2;
		int y  = n.y;
		int dy = n.dy;
		
		int x = x1;
		if (y < 0) continue;
		if (y >= h) continue;
		
		uint32_t * mapRow = colorMap + y*lineLen;
		uint32_t * fillRow = fillMap + y*lineLen;
		
		if ((x >= 0) && (x < w) && (mapRow[(x) / 32] & (1 << ((x) % 32))) )
		{
			while ((x >= 1) && (mapRow[(x-1) / 32] & (1 << ((x-1) % 32))) )
			{
				fillRow[(x-1) / 32] |= (1 << ((x-1) % 32));
				mapRow[(x-1) / 32] ^= (1 << ((x-1) % 32));
				x = x - 1;
			}
			if (x < x1)
			{
				s.push({x, x1 - 1, y - dy, -dy});
			}
		}
		
		while (x1 <= x2)
		{
			while ((x1 >= 0) && (x1 < w) && (mapRow[(x1) / 32] & (1 << ((x1) % 32))) )
			{
				fillRow[(x1) / 32] |= (1 << ((x1) % 32));
				mapRow[(x1) / 32] ^= (1 << ((x1) % 32));
				x1++;
			}
			
			if (x1 > x)
			{
				s.push({x, x1 - 1, y + dy, dy});
			}
			
			if (x1 - 1 > x2)
			{
				s.push({x2 + 1, x1 - 1, y - dy, -dy});
			}
			
			x1++;
			
			while (x1 <= x2 && !((x1 >= 0) && (x1 < w) && (mapRow[(x1) / 32] & (1 << ((x1) % 32))) ))
			{
				x1++;
			}
			
			x = x1;
		}
	}
	
	if (img.format() == QImage::Format_RGB32)
	{
		#pragma omp parallel for schedule(static, 1)
		for (int y = 0; y < h; y++)
		{
			QRgb * row = reinterpret_cast<QRgb *>(img.scanLine(y));
			uint32_t * fillRow = fillMap + y*lineLen;
			for (int x = 0; x < w; x++)
			{
				if (fillRow[(x) / 32] & (1 << ((x) % 32)))
				{
					row[x] = fillRgb;
				}
			}
		}
	} else
	if (img.format() == QImage::Format_ARGB32)
	{
		#pragma omp parallel for schedule(static, 1)
		for (int y = 0; y < h; y++)
		{
			QRgb * row = reinterpret_cast<QRgb *>(img.scanLine(y));
			uint32_t * fillRow = fillMap + y*lineLen;
			for (int x = 0; x < w; x++)
			{
				if (fillRow[(x) / 32] & (1 << ((x) % 32)))
				{
					row[x] = qRgba(qRed(fillRgb), qGreen(fillRgb), qBlue(fillRgb), qAlpha(row[x]));
				}
			}
		}
	} else
	if (hasAlpha)
	{
		#pragma omp parallel for schedule(static, 1)
		for (int y = 0; y < h; y++)
		{
			uint32_t * fillRow = fillMap + y*lineLen;
			for (int x = 0; x < w; x++)
			{
				if (fillRow[(x) / 32] & (1 << ((x) % 32)))
				{
					QColor fillColorA = fillColor;
					fillColorA.setAlpha(img.pixelColor(x, y).alpha());
					img.setPixelColor(x, y, fillColorA);
				}
			}
		}
	}
	else
	{
		#pragma omp parallel for schedule(static, 1)
		for (int y = 0; y < h; y++)
		{
			uint32_t * fillRow = fillMap + y*lineLen;
			for (int x = 0; x < w; x++)
			{
				if (fillRow[(x) / 32] & (1 << ((x) % 32)))
				{
					img.setPixelColor(x, y, fillColor);
				}
			}
		}
	}
	
	delete [] colorMap;
	delete [] fillMap;
}

void DrawDeviceFill::mousePressEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(modifiers);
	if ((buttons & Qt::LeftButton) || (buttons & Qt::RightButton))
	{
		if ((buttons & Qt::LeftButton))
		{
			fillColor = parameters->foregroundColor;
		}
		else
		{
			fillColor = parameters->backgroundColor;
		}
		QPoint intPosition = QPoint(position.x(), position.y());
		mainWindow->saveToUndoStack();
		QImage & img = parameters->display->getImageRef();
		fill(img, intPosition, parameters->tolerance);
		parameters->display->updateInternalImage();
	}
}

void DrawDeviceFill::mouseMoveEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(position);
	Q_UNUSED(buttons);
	Q_UNUSED(modifiers);
}

void DrawDeviceFill::mouseReleaseEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers)
{
	Q_UNUSED(position);
	Q_UNUSED(buttons);
	Q_UNUSED(modifiers);
}

