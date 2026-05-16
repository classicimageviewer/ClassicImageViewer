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


#ifndef DRAWDEVICES_H
#define DRAWDEVICES_H

#include "displaywidget.h"

class DrawDeviceParameters : public QObject
{
protected:
	Q_OBJECT
public:
	QColor foregroundColor, backgroundColor;
	bool antialiasing;
	int size;
	int width;
	Qt::PenStyle lineStyle;
	Qt::PenCapStyle lineEnd;
	Qt::PenJoinStyle lineJoin;
	int polygonFillOutline;
	int tolerance;
	int radius;
	QString text;
	QFont font;
	int fontSize;
	bool boldFont, italicFont;
	DisplayWidget * display;
};

class DrawDevice : public QObject
{
	Q_OBJECT
protected:
	MainWindow * mainWindow;
	DisplaySurface * surface;
	DrawDeviceParameters * parameters;
	int specialization;
public:
	DrawDevice(MainWindow * mainWindow, DrawDeviceParameters * parameters, int specialization);
	void registerSurface(DisplaySurface * surface);
	void disconnectSurface(void);
	virtual void abortDraw(void) = 0;
	virtual void mousePressEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) = 0;
	virtual void mouseMoveEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) = 0;
	virtual void mouseReleaseEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) = 0;
};



class DrawDevicePencil : public DrawDevice
{
private:
	QColor color;
	uint32_t button;
	QPoint previousPos;
	bool drawLine(QImage & img, QPoint begin, QPoint end);
public:
	DrawDevicePencil(MainWindow * mainWindow, DrawDeviceParameters * parameters, int specialization);
	virtual void abortDraw(void) override;
	virtual void mousePressEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
	virtual void mouseMoveEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
	virtual void mouseReleaseEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
};

class DrawDeviceBrush : public DrawDevice
{
private:
	QPainterPath path;
	QGraphicsPathItem * pathItem;
	QColor color;
	uint32_t button;
public:
	DrawDeviceBrush(MainWindow * mainWindow, DrawDeviceParameters * parameters, int specialization);
	virtual void abortDraw(void) override;
	virtual void mousePressEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
	virtual void mouseMoveEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
	virtual void mouseReleaseEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
};

class DrawDeviceSmudge : public DrawDevice
{
private:
	QGraphicsEllipseItem * sourceCircle, * sourceCircleDecor;
	QPoint previousPos;
	QImage sourceImg;
	QImage maskImg;
	QImage patchMask;
	QRect patchRect;
	float * deltaXmap;
	float * deltaYmap;
	int deltaMapSize;
	uint32_t button;
	bool warpMode;
	bool warpBlurMode;
	void drawTargetCircle(QPoint position);
	void copyPatch(QImage & img, QPoint position);
public:
	DrawDeviceSmudge(MainWindow * mainWindow, DrawDeviceParameters * parameters, int specialization);
	virtual void abortDraw(void) override;
	virtual void mousePressEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
	virtual void mouseMoveEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
	virtual void mouseReleaseEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
};

class DrawDeviceClone : public DrawDevice
{
private:
	int state;
	QGraphicsEllipseItem * sourceCircle, * sourceCircleDecor;
	QPoint sourcePos;
	QPoint deltaPos;
	QImage sourceImg;
	QImage maskImg;
	QImage patchMask;
	QRect patchRect;
	void drawSourceCircle(void);
	void copyPatch(QImage & img, QPoint src, QPoint dst);
public:
	DrawDeviceClone(MainWindow * mainWindow, DrawDeviceParameters * parameters, int specialization);
	virtual void abortDraw(void) override;
	virtual void mousePressEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
	virtual void mouseMoveEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
	virtual void mouseReleaseEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
};

class DrawDeviceLine : public DrawDevice
{
private:
	QGraphicsLineItem * lineItem;
	QLineF line;
	QColor color;
	uint32_t button;
public:
	DrawDeviceLine(MainWindow * mainWindow, DrawDeviceParameters * parameters, int specialization);
	virtual void abortDraw(void) override;
	virtual void mousePressEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
	virtual void mouseMoveEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
	virtual void mouseReleaseEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
};

class DrawDeviceShapes : public DrawDevice
{
private:
	QGraphicsRectItem * rectItem;
	QGraphicsPathItem * roundedRectItem;
	QGraphicsEllipseItem * ellipseItem;
	QRectF rect;
	QColor color, fillColor;
	uint32_t button;
public:
	DrawDeviceShapes(MainWindow * mainWindow, DrawDeviceParameters * parameters, int specialization);
	virtual void abortDraw(void) override;
	virtual void mousePressEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
	virtual void mouseMoveEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
	virtual void mouseReleaseEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
	enum Specialization
	{
		Rectangle,
		RoundedRectangle,
		Ellipse
	};
};

class DrawDeviceColorPicker : public DrawDevice
{
private:
	uint32_t button;
public:
	DrawDeviceColorPicker(MainWindow * mainWindow, DrawDeviceParameters * parameters, int specialization);
	virtual void abortDraw(void) override;
	virtual void mousePressEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
	virtual void mouseMoveEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
	virtual void mouseReleaseEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
};


class DrawDeviceText : public DrawDevice
{
private:
	QGraphicsTextItem * textItem;
	uint32_t button;
	QColor color;
public:
	DrawDeviceText(MainWindow * mainWindow, DrawDeviceParameters * parameters, int specialization);
	virtual void abortDraw(void) override;
	virtual void mousePressEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
	virtual void mouseMoveEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
	virtual void mouseReleaseEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
};

class DrawDeviceFill : public DrawDevice
{
private:
	struct Span
	{
		int x1, x2, y, dy;
	};
	QColor fillColor;
	void fill(QImage & img, QPoint source, int tolerance);
public:
	DrawDeviceFill(MainWindow * mainWindow, DrawDeviceParameters * parameters, int specialization);
	virtual void abortDraw(void) override;
	virtual void mousePressEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
	virtual void mouseMoveEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
	virtual void mouseReleaseEvent(const QPointF & position, const Qt::MouseButtons  buttons, const Qt::KeyboardModifiers modifiers) override;
};

#endif //DRAWDEVICES_H
