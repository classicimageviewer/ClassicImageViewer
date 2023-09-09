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


#ifndef DISPLAYWIDGET_H
#define DISPLAYWIDGET_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPixmapItem>
#include <QTimer>

class DisplayCanvas;
class DisplaySurface;
class DisplayWidget;
class MainWindow;

// MOC does not support nested classes >(


class DisplayCanvas : public QObject, public QGraphicsPixmapItem
{
	Q_OBJECT
private: // typedefs
private: // variables
	static int objCntr;
	DisplaySurface * surface;
private: // functions
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
	void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
	
public:
	DisplayCanvas(const QPixmap &pixmap, DisplaySurface * surface, QGraphicsItem *parent = NULL);
	~DisplayCanvas();
};




class DisplaySurface : public QGraphicsScene
{
	Q_OBJECT
	friend class DisplayCanvas;
private: // typedefs
	enum MouseInteractionMode {NONE=0, DRAG, SELECT, RESIZE, RESIZE_TL, RESIZE_T, RESIZE_TR, RESIZE_R, RESIZE_BR, RESIZE_B, RESIZE_BL, RESIZE_L, MOVE};

private: // variables
	static int objCntr;
	DisplayWidget * parent;
	QTimer scrollTimer;
	QImage image;
	QRect imageRect;
	QImage canvasImage;
	DisplayCanvas * canvas;
	double zoom;
	bool selectionEnabled;
	bool selectionVisible;
	QRect selection;
	MouseInteractionMode mouseInteraction;
	QPoint mouseStartPoint, mouseEndPoint;
	QRect sensorC, sensorTL, sensorT, sensorTR, sensorR, sensorBR, sensorB, sensorBL, sensorL;
	QRect selectionCopyForMove;
	int selectScrollX, selectScrollY;
	bool drawnWithoutSelection;
	QPoint pixelInfoPos;
private: // functions
	void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
	void changeSelection(QRect selection);
private slots:
	void onScrollTimeout();
public:
	DisplaySurface(DisplayWidget * parent);
	~DisplaySurface();
	
	void hoverAction(QPoint pos);
	void hoverLeaveAction(QPoint pos);
	void setImage(const QImage &image);
	void redraw();
	QRect getSelection();
	void setZoom(double zoom);
	double getZoom();
	void enableSelection(bool enable);
	QPoint getPixelInfoPos();
signals:
	void zoomChanged();
	void selectionChanged();
	void pixelInfo();
};




class DisplayWidget : public QGraphicsView
{
	Q_OBJECT
	friend class DisplaySurface;
private: // typedefs
private: // variables
	MainWindow * parent;
	QGraphicsScene * emptyScene;
	DisplaySurface * surface;
	double zoom;
	QImage image;
	bool selectionEnabled;
private: // functions
	bool eventFilter(QObject* watched, QEvent* event);
	void wheelEvent(QWheelEvent *event) override;
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dragMoveEvent(QDragMoveEvent* event) override;
	void dragLeaveEvent(QDragLeaveEvent* event) override;
	void dropEvent(QDropEvent* event) override;
public:
	DisplayWidget(QWidget *parent = NULL);
	~DisplayWidget();
	
	void setBackgroundShade(int shade);
	void newImage(const QImage &image);
	void updateImage(const QImage &image);
	void insertIntoSelection(const QImage &image);
	QSize getImageSize();
	QImage getImage();
	QImage getFromSelection();
	QRect getSelection();
	QSize getViewportSize();
	void enableSelection(bool enable);
	void setZoom(double zoom);
	double getZoom();
	QColor getPixelInfoColor();
	bool getPixelInfoHasAlpha();
	QPoint getPixelInfoPos();
signals:
	void needNextImage();
	void needPrevImage();
	void needFirstImage();
	void needLastImage();
	void zoomChanged();
	void selectionChanged();
	void pixelInfo();
};


#endif //DISPLAYWIDGET_H
