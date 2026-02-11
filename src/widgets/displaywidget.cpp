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


#include "displaywidget.h"
#include "globals.h"
#include <cmath>
#include <QDebug>
#include <QPainter>
#include <QScrollBar>
#include <QGraphicsSceneMouseEvent>
#include <QWheelEvent>

int DisplaySurface::objCntr = 0;
int DisplayCanvas::objCntr = 0;


DisplayWidget::DisplayWidget(QWidget *parent) : QGraphicsView(parent)
{
	setBackgroundBrush(QBrush(Qt::black));
	setSizePolicy(QSizePolicy::Expanding , QSizePolicy::Expanding);
	emptyScene = new QGraphicsScene(this);
	surface = NULL;
	selectionEnabled = true;
	zoom = 1.0;
	image = QImage();
	mousePositionCorrection = QPointF(0, 0);
	setScene(emptyScene);	// dummy
	setMouseTracking(true);
	setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
	this->parent = (MainWindow*)parent;
	this->installEventFilter(this);
}

DisplayWidget::~DisplayWidget()
{
	setScene(NULL);
	if (surface)
	{
		disconnect(surface, SIGNAL(zoomChanged()), NULL, NULL);
		disconnect(surface, SIGNAL(selectionChanged()), NULL, NULL);
		delete surface;
	}
	delete emptyScene;
}

void DisplayWidget::setBackgroundShade(int shade)
{
	// shade 0 .. 100
	shade *= 255;
	shade /= 100;
	setBackgroundBrush(QBrush(QColor(shade, shade, shade)));
}

void DisplayWidget::newImage(const QImage &image)
{
	if (surface)
	{
		setScene(NULL);
		disconnect(surface, SIGNAL(zoomChanged()), NULL, NULL);
		disconnect(surface, SIGNAL(selectionChanged()), NULL, NULL);
		disconnect(surface, SIGNAL(pixelInfo()), NULL, NULL);
		delete surface;
		surface = NULL;
	}
	this->image = image;
	if (image.isNull())
	{
		emit zoomChanged();
		emit selectionChanged();
		setScene(emptyScene);
		return;
	}
	surface = new DisplaySurface(this);
	connect(surface, SIGNAL(zoomChanged()), this, SIGNAL(zoomChanged())); // redirect signals
	connect(surface, SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()));
	connect(surface, SIGNAL(pixelInfo()), this, SIGNAL(pixelInfo()));
	surface->enableSelection(selectionEnabled);
	setScene(surface);
	surface->setImage(this->image);
	surface->setZoom(zoom);
	emit selectionChanged();
}

void DisplayWidget::updateImage(const QImage &image)
{
	if (!surface) return newImage(image);
	if (this->image.size() != image.size()) return newImage(image);
	
	this->image = image.copy();
	surface->setImage(this->image);
	emit zoomChanged();
}

void DisplayWidget::insertIntoSelection(const QImage &image)
{
	if (!surface) return;
	QRect selection = surface->getSelection();
	if (selection.isNull()) return;
	QImage scaledImage = image.scaled(selection.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	QPainter painter(&this->image);
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	painter.drawImage(selection.topLeft(), scaledImage);
	painter.end();
	surface->setImage(this->image);
}

QSize DisplayWidget::getImageSize()
{
	if (!surface) return QSize(0,0);
	return image.size();
}

int DisplayWidget::getImageBpp()
{
	if (!surface) return 0;
	return image.bitPlaneCount();
}

QImage DisplayWidget::getImage()
{
	if (!surface) return QImage();
	return image;
}

QImage DisplayWidget::getFromSelection()
{
	if (!surface) return QImage();
	QRect selection = surface->getSelection();
	if (selection.isNull()) return image;
	return image.copy(selection);
}

QRect DisplayWidget::getSelection()
{
	if (!surface) return QRect();
	return surface->getSelection();
}

void DisplayWidget::setSelection(QRect newSelection)
{
	if (!surface) return;
	surface->setSelection(newSelection);
}

QSize DisplayWidget::getViewportSize()
{
	return (viewport()->size() * Globals::scalingFactor);
}

void DisplayWidget::enableSelection(bool enable)
{
	selectionEnabled = enable;
	if (surface)
	{
		surface->enableSelection(selectionEnabled);
	}
}

void DisplayWidget::setZoom(double zoom)
{
	if (!surface) return;
	if (zoom <= 0) return;
	if (zoom > 1000.0) zoom = 1000.0;
	if (zoom < 0.001) zoom = 0.001;
	this->zoom = zoom;
	resetTransform();
	scale(zoom, zoom);
	surface->setZoom(zoom);
}

double DisplayWidget::getZoom()
{
	if (!surface) return 1.0;
	return surface->getZoom();
}

QColor DisplayWidget::getPixelInfoColor()
{
	if (!surface) return QColor();
	return QColor(image.pixel(surface->getPixelInfoPos()));
}

bool DisplayWidget::getPixelInfoHasAlpha()
{
	return image.hasAlphaChannel();
}

QPoint DisplayWidget::getPixelInfoPos()
{
	if (!surface) return QPoint();
	return surface->getPixelInfoPos();
}

QPointF DisplayWidget::getMousePositionCorrection()
{
	return mousePositionCorrection;
}

bool DisplayWidget::eventFilter(QObject* watched, QEvent* event)
{
	QKeyEvent * keyEvent = (QKeyEvent*)event;
	if (event->type() == QEvent::KeyPress)
	{
		//if (keyEvent->key() == Qt::Key_Return)
		if (verticalScrollBar()->isVisible())
		{
			if ((keyEvent->modifiers() == Qt::NoModifier) && (keyEvent->key() == Qt::Key_Home))
			{
				verticalScrollBar()->setValue(0);
				return true;
			}
			if ((keyEvent->modifiers() == Qt::NoModifier) && (keyEvent->key() == Qt::Key_End))
			{
				verticalScrollBar()->setValue(verticalScrollBar()->maximum());
				return true;
			}
			if ((keyEvent->modifiers() == Qt::NoModifier) && (keyEvent->key() == Qt::Key_PageUp))
			{
				verticalScrollBar()->setValue(verticalScrollBar()->value() - verticalScrollBar()->pageStep());
				return true;
			}
			if ((keyEvent->modifiers() == Qt::NoModifier) && (keyEvent->key() == Qt::Key_PageDown))
			{
				verticalScrollBar()->setValue(verticalScrollBar()->value() + verticalScrollBar()->pageStep());
				return true;
			}
		}
		if (horizontalScrollBar()->isVisible())
		{
			if ((keyEvent->modifiers() == Qt::ShiftModifier) && (keyEvent->key() == Qt::Key_Home))
			{
				horizontalScrollBar()->setValue(0);
				return true;
			}
			if ((keyEvent->modifiers() == Qt::ShiftModifier) && (keyEvent->key() == Qt::Key_End))
			{
				horizontalScrollBar()->setValue(horizontalScrollBar()->maximum());
				return true;
			}
			if ((keyEvent->modifiers() == Qt::ShiftModifier) && (keyEvent->key() == Qt::Key_PageUp))
			{
				horizontalScrollBar()->setValue(horizontalScrollBar()->value() - horizontalScrollBar()->pageStep());
				return true;
			}
			if ((keyEvent->modifiers() == Qt::ShiftModifier) && (keyEvent->key() == Qt::Key_PageDown))
			{
				horizontalScrollBar()->setValue(horizontalScrollBar()->value() + horizontalScrollBar()->pageStep());
				return true;
			}
		}
		if (!verticalScrollBar()->isVisible())
		{
			if (((keyEvent->modifiers() == Qt::NoModifier) && (keyEvent->key() == Qt::Key_Down)) || (keyEvent->key() == Qt::Key_PageDown))
			{
				emit needNextImage();
				return true;
			}
			if (((keyEvent->modifiers() == Qt::NoModifier) && (keyEvent->key() == Qt::Key_Up)) || (keyEvent->key() == Qt::Key_PageUp))
			{
				emit needPrevImage();
				return true;
			}
			if (keyEvent->key() == Qt::Key_Home)
			{
				emit needFirstImage();
				return true;
			}
			if (keyEvent->key() == Qt::Key_End)
			{
				emit needLastImage();
				return true;
			}
		}
		if (!horizontalScrollBar()->isVisible())
		{
			if ((keyEvent->modifiers() == Qt::NoModifier) && (keyEvent->key() == Qt::Key_Right))
			{
				emit needNextImage();
				return true;
			}
			if ((keyEvent->modifiers() == Qt::NoModifier) && (keyEvent->key() == Qt::Key_Left))
			{
				emit needPrevImage();
				return true;
			}
		}
		if ((keyEvent->modifiers() == Qt::ControlModifier) && (keyEvent->key() == Qt::Key_A))
		{
			if (surface)
			{
				surface->toggleSelectionAll();
				return true;
			}
		}
		
		if (keyEvent->modifiers() == Qt::ShiftModifier)
		{
			if (surface)
			{
				if (keyEvent->key() == Qt::Key_Left)
				{
					surface->adjustSelection(-1, 0, -1, 0);
					return true;
				}
				if (keyEvent->key() == Qt::Key_Up)
				{
					surface->adjustSelection(0, -1, 0, -1);
					return true;
				}
				if (keyEvent->key() == Qt::Key_Right)
				{
					surface->adjustSelection(1, 0, 1, 0);
					return true;
				}
				if (keyEvent->key() == Qt::Key_Down)
				{
					surface->adjustSelection(0, 1, 0, 1);
					return true;
				}
			}
		}
		if (keyEvent->modifiers() == Qt::ControlModifier)
		{
			if (surface)
			{
				if (keyEvent->key() == Qt::Key_Left)
				{
					surface->adjustSelection(-1, 0, 0, 0);
					return true;
				}
				if (keyEvent->key() == Qt::Key_Up)
				{
					surface->adjustSelection(0, -1, 0, 0);
					return true;
				}
				if (keyEvent->key() == Qt::Key_Right)
				{
					surface->adjustSelection(1, 0, 0, 0);
					return true;
				}
				if (keyEvent->key() == Qt::Key_Down)
				{
					surface->adjustSelection(0, 1, 0, 0);
					return true;
				}
			}
		}
		if (keyEvent->modifiers() == Qt::AltModifier)
		{
			if (surface)
			{
				if (keyEvent->key() == Qt::Key_Left)
				{
					surface->adjustSelection(0, 0, -1, 0);
					return true;
				}
				if (keyEvent->key() == Qt::Key_Up)
				{
					surface->adjustSelection(0, 0, 0, -1);
					return true;
				}
				if (keyEvent->key() == Qt::Key_Right)
				{
					surface->adjustSelection(0, 0, 1, 0);
					return true;
				}
				if (keyEvent->key() == Qt::Key_Down)
				{
					surface->adjustSelection(0, 0, 0, 1);
					return true;
				}
			}
		}
	}
	return QObject::eventFilter(watched, event);
}

void DisplayWidget::wheelEvent(QWheelEvent *event)
{
	int direction = ((event->angleDelta().y() > 0) ? 1:-1);
	if (Globals::prefs->getReverseWheel())
	{
		direction *= -1;
	}
	
	if (event->modifiers() == Qt::ControlModifier)
	{
		double zoomDelta = Globals::prefs->getZoomDelta();
		zoomDelta = (zoomDelta/100.0) + 1.0;
#if QT_VERSION < QT_VERSION_CHECK(5,14,0)
		QPoint mousePos = event->pos();
#else
		QPoint mousePos = event->position().toPoint();
#endif
		QPointF posBeforeScale = mapToScene(mousePos);
		if (direction > 0)
		{
			setZoom(getZoom() * zoomDelta);
		}
		else
		{
			setZoom(getZoom() / zoomDelta);
		}
		QPointF posAfterScale = mapFromScene(posBeforeScale);
		QPointF delta = posAfterScale - mousePos;
		horizontalScrollBar()->setValue(horizontalScrollBar()->value() + delta.x());
		verticalScrollBar()->setValue(verticalScrollBar()->value() + delta.y());
	}
	else
	if ((event->modifiers() == Qt::ShiftModifier) && (horizontalScrollBar()->isVisible()))
	{
		horizontalScrollBar()->setValue(horizontalScrollBar()->value() - QApplication::wheelScrollLines()*direction*horizontalScrollBar()->singleStep());
	}
	else
	if ((event->modifiers() == Qt::NoModifier) && (verticalScrollBar()->isVisible()))
	{
		verticalScrollBar()->setValue(verticalScrollBar()->value() - QApplication::wheelScrollLines()*direction*verticalScrollBar()->singleStep());
	}
	else
	if ((event->modifiers() == Qt::NoModifier) && (!verticalScrollBar()->isVisible()))
	{
		if (direction > 0)
		{
			emit needNextImage();
		}
		else
		{
			emit needPrevImage();
		}
	}
	else
	{
		QGraphicsView::wheelEvent(event);
	}
}

void DisplayWidget::dragEnterEvent(QDragEnterEvent* event)
{
	parent->dragEnterEvent(event);
}

void DisplayWidget::dragMoveEvent(QDragMoveEvent* event)
{
	parent->dragMoveEvent(event);
}

void DisplayWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
	parent->dragLeaveEvent(event);
}

void DisplayWidget::dropEvent(QDropEvent* event)
{
	parent->dropEvent(event);
}

void DisplayWidget::mouseMoveEvent(QMouseEvent *event)
{
	double tmp;
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
	mousePositionCorrection = QPointF(std::modf(event->windowPos().x(), &tmp), std::modf(event->windowPos().y(), &tmp)) * -1.0;
#else
	mousePositionCorrection = QPointF(std::modf(event->scenePosition().x(), &tmp), std::modf(event->scenePosition().y(), &tmp)) * -1.0;
#endif
	QGraphicsView::mouseMoveEvent(event);
}



FastSelector::FastSelector(DisplaySurface * surface)
{
	mask = new QGraphicsPathItem();
	frameBase = new QGraphicsRectItem();
	frameDecor = new QGraphicsRectItem();
	mask->setVisible(false);
	frameBase->setVisible(false);
	frameDecor->setVisible(false);
	surface->addItem(mask);
	surface->addItem(frameBase);
	surface->addItem(frameDecor);
}

FastSelector::~FastSelector()
{
	// items are deleted by the surface
}

void FastSelector::setVisible(bool visible)
{
	mask->setVisible(visible);
	frameBase->setVisible(visible);
	frameDecor->setVisible(visible);
}

void FastSelector::drawSelection(QRect imageRect, QRect selection, double width)
{
	QPainterPath path, path2;
	path.addRect(QRectF(QPointF(0, 0), (imageRect.size() / Globals::scalingFactor)));
	QRectF adjustedRect = QRectF((QPointF(selection.topLeft()) + QPointF(0.5, 0.5)) / Globals::scalingFactor, (QPointF(selection.bottomRight()) + QPointF(0.5, 0.5)) / Globals::scalingFactor);
	path2.addRect(adjustedRect);

	mask->setPath(path.subtracted(path2));
	frameBase->setRect(adjustedRect);
	frameDecor->setRect(adjustedRect);
	
	mask->setPen(Qt::NoPen);
	mask->setBrush(QBrush(QColor(128,128,128,128)));
	
	QPen penBase = QPen(QBrush(), width, Qt::SolidLine);
	penBase.setColor(QColor(0,0,0,128));
	frameBase->setPen(penBase);
	
	QPen penDecor = QPen(QBrush(), width, Qt::CustomDashLine);
	penDecor.setColor(Qt::white);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
	QVector<qreal> dashes;
#else
	QList<qreal> dashes;
#endif
	dashes << 5 << 6;
	penDecor.setDashPattern(dashes);
	frameDecor->setPen(penDecor);
}




DisplaySurface::DisplaySurface(DisplayWidget * parent) : QGraphicsScene(parent)
{
	this->parent = parent;
	imageRect = QRect();
	image = QImage();
	canvas = NULL;
	useFastSelector = Globals::prefs->getUseFastSelector();
	fastSelector = NULL;
	selectionEnabled = false;
	selectionVisible = false;
	selectionAgain = false;
	selection = QRect();
	zoom = 1.0;
	mouseInteraction = NONE;
	selectScrollX = selectScrollY = 0;
	drawnWithoutSelection = false;
	connect(&scrollTimer, SIGNAL(timeout()), this, SLOT(onScrollTimeout()));
	scrollTimer.start(33);
	if (objCntr) qDebug() << "DisplaySurface::objCntr != 0";
	objCntr++;
}

DisplaySurface::~DisplaySurface()
{
	parent->unsetCursor();
	scrollTimer.stop();
	delete canvas;
	delete fastSelector;
	objCntr--;
}

void DisplaySurface::onScrollTimeout()
{
	int speedUp = 2;
	int maxSpeed = 64;
	if (selectScrollX)
	{
		int amount = selectScrollX * speedUp;
		if (amount > maxSpeed) amount = maxSpeed;
		if (amount < maxSpeed*-1) amount = maxSpeed*-1;
		parent->horizontalScrollBar()->setValue(parent->horizontalScrollBar()->value() + amount);
	}
	if (selectScrollY)
	{
		int amount = selectScrollY * speedUp;
		if (amount > maxSpeed) amount = maxSpeed;
		if (amount < maxSpeed*-1) amount = maxSpeed*-1;
		parent->verticalScrollBar()->setValue(parent->verticalScrollBar()->value() + amount);
	}
}

void DisplaySurface::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	mouseInteraction = NONE;
	mouseOffset = QPoint();
	if(event->button() == Qt::LeftButton)
	{
		if (selectionEnabled)
		{
			QPoint pos = mousePositionInImage(event);
			mouseStartPoint = pos;
			mouseEndPoint = pos;
			mouseInteraction = SELECT;
			if (selectionVisible)
			{
				if (sensorTL.contains(pos))
				{
					mouseInteraction = RESIZE_TL;
				} else
				if (sensorTR.contains(pos))
				{
					mouseInteraction = RESIZE_TR;
				} else
				if (sensorBR.contains(pos))
				{
					mouseInteraction = RESIZE_BR;
				} else
				if (sensorBL.contains(pos))
				{
					mouseInteraction = RESIZE_BL;
				} else
				if (sensorT.contains(pos))
				{
					mouseInteraction = RESIZE_T;
				} else
				if (sensorR.contains(pos))
				{
					mouseInteraction = RESIZE_R;
				} else
				if (sensorB.contains(pos))
				{
					mouseInteraction = RESIZE_B;
				} else
				if (sensorL.contains(pos))
				{
					mouseInteraction = RESIZE_L;
				} else
				if (sensorC.contains(pos))
				{
					mouseInteraction = MOVE;
					selectionCopyForMove = selection;
					mouseOffset = pos - selection.center();
				}
				if (selection.height() > 0)
				{
					selectionAspectRatio = static_cast<double>(selection.width()) / selection.height();
				}
				else
				{
					selectionAspectRatio = 0.0;
				}
			}
			selectionAgain = (mouseInteraction == SELECT) && selectionVisible;
			selectionVisible = true;
			if (mouseInteraction == SELECT)
			{
				parent->setCursor(Qt::CrossCursor);
			}
			
			if (image.rect().contains(pos))
			{
				pixelInfoPos = pos;
				emit pixelInfo();
			}
		}
	}
	else
	if(event->button() == Qt::RightButton)
	{
		QPoint pos = mousePositionInImage(event);
		bool moveSelection = selectionEnabled && selectionVisible && selection.contains(pos);
		if (moveSelection)
		{
			// check the selection is visible in the viewport
			QRectF visibleRectInSurface = parent->mapToScene(parent->viewport()->rect()).boundingRect() ;
			QRectF scaledSelection = QRectF(selection.x() / Globals::scalingFactor, selection.y() / Globals::scalingFactor, selection.width() / Globals::scalingFactor, selection.height() / Globals::scalingFactor);
			moveSelection = visibleRectInSurface.contains(scaledSelection);
		}
		if (moveSelection)
		{
			mouseStartPoint = pos;
			mouseEndPoint = pos;
			mouseOffset = pos - selection.center();
			mouseInteraction = MOVE;
			selectionCopyForMove = selection;
			selectionAgain = false;
			parent->setCursor(Qt::ClosedHandCursor);
		}
		else
		{
			QPoint pos = parent->mapFromScene(event->scenePos());
			mouseStartPoint = pos;
			mouseEndPoint = pos;
			mouseInteraction = DRAG;
			parent->setCursor(Qt::ClosedHandCursor);
		}
	}
	selectScrollX = selectScrollY = 0;
	QGraphicsScene::mousePressEvent(event);
}

void DisplaySurface::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if((event->buttons() & Qt::LeftButton) || (event->buttons() & Qt::RightButton))
	{
		QPoint pos = mousePositionInImage(event);
		mouseEndPoint = pos - mouseOffset;
		QRect selectionCopy = selection;
		if (mouseInteraction == DRAG)
		{
			mouseEndPoint = parent->mapFromScene(event->scenePos());
			QPoint delta = mouseEndPoint - mouseStartPoint;
			mouseStartPoint = mouseEndPoint;
			parent->horizontalScrollBar()->setValue(parent->horizontalScrollBar()->value() - delta.x());
			parent->verticalScrollBar()->setValue(parent->verticalScrollBar()->value() - delta.y());
		} else
		if (mouseInteraction == SELECT)
		{
			changeSelection(QRect(mouseStartPoint, mouseEndPoint));
			
			selectScrollX = selectScrollY = 0;
			QPoint cursorPos = parent->mapFromScene(event->scenePos());
			int scrollCaptureRange = 8*Globals::scalingFactor;	// TODO configurable?
			if(cursorPos.x() < scrollCaptureRange) {
				selectScrollX = cursorPos.x() - scrollCaptureRange;
			}
			else if(parent->width()-cursorPos.x() < scrollCaptureRange) {
				selectScrollX = cursorPos.x() - parent->width() + scrollCaptureRange;
			}
			if(cursorPos.y() < scrollCaptureRange) {
				selectScrollY = cursorPos.y() - scrollCaptureRange;
			}
			else if(parent->height()-cursorPos.y() < scrollCaptureRange) {
				selectScrollY = cursorPos.y() - parent->height() + scrollCaptureRange;
			}
		} else
		if (mouseInteraction == MOVE)
		{
			selectionCopy = selectionCopyForMove;
			selectionCopy.moveCenter(mouseEndPoint);
			QRect imageRect = image.rect();
			if (!imageRect.contains(selectionCopy))	// limit the move within the boundaries of the image
			{
				int boundedX = selectionCopy.x();
				int boundedY = selectionCopy.y();
				if (boundedX < 0) boundedX = 0;
				if (boundedY < 0) boundedY = 0;
				if ((imageRect.width() - selectionCopy.width()) < boundedX)
				{
					boundedX = (imageRect.width() - selectionCopy.width());
				}
				if ((imageRect.height() - selectionCopy.height()) < boundedY)
				{
					boundedY = (imageRect.height() - selectionCopy.height());
				}
				selectionCopy.moveTo(QPoint(boundedX, boundedY));
			}
			changeSelection(selectionCopy);
		} else
		if (mouseInteraction >= RESIZE)
		{
			switch(mouseInteraction)
			{
				case RESIZE_TL: selectionCopy.setTopLeft(mouseEndPoint);     break;
				case RESIZE_T : selectionCopy.setTop(mouseEndPoint.y());     break;
				case RESIZE_TR: selectionCopy.setTopRight(mouseEndPoint);    break;
				case RESIZE_R : selectionCopy.setRight(mouseEndPoint.x());   break;
				case RESIZE_BR: selectionCopy.setBottomRight(mouseEndPoint); break;
				case RESIZE_B : selectionCopy.setBottom(mouseEndPoint.y());  break;
				case RESIZE_BL: selectionCopy.setBottomLeft(mouseEndPoint);  break;
				case RESIZE_L : selectionCopy.setLeft(mouseEndPoint.x());    break;
				default: break;
			}
			
			if ((event->modifiers() == Qt::ControlModifier) && (selectionAspectRatio > 0.0))
			{
				QRect normalizedSelection = image.rect().intersected(selectionCopy).normalized();
				int w = normalizedSelection.width();
				int h = normalizedSelection.height();
				
				if ((h > 0) && (w > 0))
				{
					switch (mouseInteraction)
					{
						case RESIZE_TL:
						case RESIZE_TR:
						case RESIZE_BR:
						case RESIZE_BL:
							{
								double targetH = w / selectionAspectRatio;
								double targetW = h * selectionAspectRatio;
								if (std::abs(targetH - h) < std::abs(targetW - w))
								{
									if ((mouseInteraction == RESIZE_TL) || (mouseInteraction == RESIZE_TR))
									{
										selectionCopy.setTop(selectionCopy.bottom() - targetH);
									}
									else
									{
										selectionCopy.setBottom(selectionCopy.top() + targetH);
									}
								} else {
									if ((mouseInteraction == RESIZE_TL) || (mouseInteraction == RESIZE_BL))
									{
										selectionCopy.setLeft(selectionCopy.right() - targetW);
									}
									else
									{
										selectionCopy.setRight(selectionCopy.left() + targetW);
									}
								}
							}
							break;
						case RESIZE_T:
						case RESIZE_B:
							{
								int newH = h;
								int newW = static_cast<int>(newH * selectionAspectRatio);
								int centerX = normalizedSelection.center().x();
								selectionCopy.setLeft(centerX - newW/2);
								selectionCopy.setRight(centerX + newW/2);
							}
							break;
						case RESIZE_L:
						case RESIZE_R:
							{
								int newW = w;
								int newH = static_cast<int>(newW / selectionAspectRatio);
								int centerY = normalizedSelection.center().y();
								selectionCopy.setTop(centerY - newH/2);
								selectionCopy.setBottom(centerY + newH/2);
							}
							break;
						default:
							break;
					}
				}
			}
			
			changeSelection(selectionCopy);
		}
	}
	QGraphicsScene::mouseMoveEvent(event);
}

void DisplaySurface::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	if (mouseInteraction == SELECT)
	{
		QRect finalSelection = QRect(mouseStartPoint, mouseEndPoint).normalized();
		double slippedCursorRange = (2*Globals::scalingFactor);
		if (selectionAgain && (finalSelection.width() <= slippedCursorRange) && (finalSelection.height() <= slippedCursorRange))
		{
			changeSelection(QRect());
		} else
		if ((mouseStartPoint == mouseEndPoint) || (finalSelection.width() == 0) || (finalSelection.height() == 0))
		{
			changeSelection(QRect());
		}
		else
		{
			changeSelection(finalSelection);
		}
	} else
	if (mouseInteraction >= RESIZE)
	{
		QRect finalSelection = selection.normalized();
		if ((finalSelection.width() == 0) || (finalSelection.height() == 0))
		{
			changeSelection(QRect());
		}
		else
		{
			changeSelection(finalSelection);
		}
	}
	parent->unsetCursor();
	mouseInteraction = NONE;
	selectScrollX = selectScrollY = 0;
	selectionAgain = false;
	QGraphicsScene::mouseReleaseEvent(event);
}

void DisplaySurface::hoverAction(QPoint pos)
{
	if (!selectionEnabled) return;
	if (!selectionVisible) return;
	if (mouseInteraction == NONE)
	{
		if (sensorTL.contains(pos))
		{
			parent->setCursor(Qt::SizeFDiagCursor);
		} else
		if (sensorTR.contains(pos))
		{
			parent->setCursor(Qt::SizeBDiagCursor);
		} else
		if (sensorBR.contains(pos))
		{
			parent->setCursor(Qt::SizeFDiagCursor);
		} else
		if (sensorBL.contains(pos))
		{
			parent->setCursor(Qt::SizeBDiagCursor);
		} else
		if (sensorT.contains(pos))
		{
			parent->setCursor(Qt::SizeVerCursor);
		} else
		if (sensorR.contains(pos))
		{
			parent->setCursor(Qt::SizeHorCursor);
		} else
		if (sensorB.contains(pos))
		{
			parent->setCursor(Qt::SizeVerCursor);
		} else
		if (sensorL.contains(pos))
		{
			parent->setCursor(Qt::SizeHorCursor);
		} else
		if (sensorC.contains(pos))
		{
			parent->setCursor(Qt::SizeAllCursor);
		} else
		{
			parent->unsetCursor();
		}
	}
}

void DisplaySurface::hoverLeaveAction(QPoint pos)
{
	Q_UNUSED(pos);
	if (mouseInteraction == NONE)
	{
		parent->unsetCursor();
	}
}

void DisplaySurface::changeSelection(QRect selection)
{
	selection = selection.normalized();
	if (selection.isNull() && (selection.x() == 0) && (selection.y() == 0))
	{
		this->selection = selection;
		selectionVisible = false;
		sensorC = sensorTL = sensorT = sensorTR = sensorR = sensorBR = sensorB = sensorBL = sensorL = QRect();
	}
	else
	{
		this->selection = (selection & imageRect);	// intersection
		// set sensors
		int minSize = selection.width();
		if (selection.height() < minSize) minSize = selection.height();
		int sensorRange = 16 * Globals::scalingFactor;
		if (zoom < 1.0)
		{
			sensorRange /= zoom;
		}
		
		if (sensorRange >= (minSize/3-1))
		{
			sensorRange = minSize/3-1;
		}

		sensorC = sensorTL = sensorT = sensorTR = sensorR = sensorBR = sensorB = sensorBL = sensorL = QRect();

		if (sensorRange >= 3)
		{
			sensorRange = (sensorRange/2)*2 + 1;
			
			sensorTL = QRect(selection.topLeft(), QSize(sensorRange, sensorRange));
			sensorTL.moveCenter(selection.topLeft());
			sensorTR = QRect(selection.topRight(), QSize(sensorRange, sensorRange));
			sensorTR.moveCenter(selection.topRight());
			sensorBR = QRect(selection.bottomRight(), QSize(sensorRange, sensorRange));
			sensorBR.moveCenter(selection.bottomRight());
			sensorBL = QRect(selection.bottomLeft(), QSize(sensorRange, sensorRange));
			sensorBL.moveCenter(selection.bottomLeft());
			if (minSize/3 > 3)
			{
				sensorC = QRect(selection.center(), QSize(minSize/3, minSize/3));
				sensorC.moveCenter(selection.center());
			}
			
			sensorT = QRect(0,0, selection.width() - sensorRange, sensorRange);
			sensorT.moveCenter(QPoint(selection.center().x(), selection.y()));
			
			sensorR = QRect(0,0, sensorRange, selection.height() - sensorRange);
			sensorR.moveCenter(QPoint(selection.x()+selection.width(), selection.center().y()));
			
			sensorB = QRect(0,0, selection.width() - sensorRange, sensorRange);
			sensorB.moveCenter(QPoint(selection.center().x(), selection.y()+selection.height()));
			
			sensorL = QRect(0,0, sensorRange, selection.height() - sensorRange);
			sensorL.moveCenter(QPoint(selection.x(), selection.center().y()));
		}
	}
	redraw();
	emit selectionChanged();
}

QPoint DisplaySurface::mousePositionInImage(QGraphicsSceneMouseEvent *event)
{
	int x = event->scenePos().x()*Globals::scalingFactor+0.4999999 + getMousePositionCorrection().x();
	int y = event->scenePos().y()*Globals::scalingFactor+0.4999999 + getMousePositionCorrection().y();
	return QPoint(x, y);
}

void DisplaySurface::setImage(const QImage &image)
{
	imageRect = image.rect();
	
	if (image.hasAlphaChannel())
	{
		this->image = QImage(image.size(), QImage::Format_ARGB32_Premultiplied);
		QPainter painter(&this->image);
		
		painter.setCompositionMode(QPainter::CompositionMode_Source);
		QBrush brush;
		brush.setTextureImage(QImage(":/pixmaps/pixmaps/chesspatt.png"));
		brush.setTransform(QTransform::fromScale(Globals::scalingFactor, Globals::scalingFactor));
		painter.setBrush(brush);
		painter.fillRect(image.rect(), brush);
		
		painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
		painter.drawImage(0, 0, image);
		
		painter.end();
		this->image = this->image.convertToFormat(QImage::Format_RGB32);
	}
	else
	{
		this->image = image.convertToFormat(QImage::Format_RGB32);
	}
	this->image.setDevicePixelRatio(Globals::scalingFactor);
	if (!canvas)
	{
		canvas = new DisplayCanvas(QPixmap::fromImage(this->image), this);
		canvas->setZoom(zoom);
		addItem(canvas);
		if (useFastSelector)
		{
			fastSelector = new FastSelector(this);
		}
	}
	else
	{
		if (useFastSelector)
		{
			canvas->setCanvasPixmap(QPixmap::fromImage(this->image));
		}
	}
	drawnWithoutSelection = false;
	redraw();
}

void DisplaySurface::redraw()
{
	if (!(selectionVisible && selectionEnabled)) // only draw when necessary
	{
		if (drawnWithoutSelection) return;
		drawnWithoutSelection = true;
	}
	else
	{
		drawnWithoutSelection = false;
	}
	
	double width = 1.0;
	if (zoom < 1)
	{
		width = 1.0+1.0/zoom;
	}
	if (zoom >= Globals::scalingFactor)
	{
		width = 1.0/Globals::scalingFactor;
	}
	
	if ((zoom < 1.0) || !selectionVisible || !selectionEnabled)
		canvas->setTransformationMode(Qt::SmoothTransformation);
	else
		canvas->setTransformationMode(Qt::FastTransformation);
	
	if (useFastSelector && fastSelector)
	{
		fastSelector->setVisible(selectionVisible && selectionEnabled);
		if (selectionVisible && selectionEnabled)
		{
			fastSelector->drawSelection(image.rect(), selection, width);
		}
	}
	else
	{
		QImage canvasImage = image;	// let Qt handle data sharing / copy
		if (selectionVisible && selectionEnabled)
		{
			QPainter painter(&canvasImage);
			painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
			
			double width = 1.0;
			if (zoom < 1)
			{
				width = 1.0+1.0/zoom;
			}
			if (zoom >= Globals::scalingFactor)
			{
				width = 1.0/Globals::scalingFactor;
			}
			
			QPen pen = QPen(QBrush(), width, Qt::SolidLine);
			pen.setColor(Qt::white);
			painter.setPen(pen);
			painter.setBrush(QBrush());
			painter.drawRect(QRectF(QPointF(selection.topLeft())/Globals::scalingFactor, QPointF(selection.bottomRight())/Globals::scalingFactor));
			painter.end();
		}
		
		QPixmap pixmap = QPixmap::fromImage(canvasImage);
		canvas->setCanvasPixmap(pixmap);
	}
	
	canvas->pixmap().setDevicePixelRatio(Globals::scalingFactor);
}

QRect DisplaySurface::getSelection()
{
	if (!selectionEnabled) return QRect();
	return selection;
}

void DisplaySurface::setSelection(QRect newSelection)
{
	if (!selectionEnabled) return;
	selectionVisible = true;
	changeSelection(newSelection);
}

void DisplaySurface::setZoom(double zoom)
{
	if (zoom > 0)
	{
		this->zoom = zoom;
		if (canvas)
		{
			canvas->setZoom(zoom);
		}
		redraw();
		emit zoomChanged();
	}
}

double DisplaySurface::getZoom()
{
	return zoom;
}

void DisplaySurface::enableSelection(bool enable)
{
	selectionEnabled = enable;
	if (!selectionEnabled && selectionVisible)
	{
		selectionVisible = false;
		selection = QRect();
		redraw();
	}
	emit selectionChanged();
}

QPoint DisplaySurface::getPixelInfoPos()
{
	return pixelInfoPos;
}

QPointF DisplaySurface::getMousePositionCorrection()
{
	return parent->getMousePositionCorrection();
}

void DisplaySurface::toggleSelectionAll()
{
	if (selectionEnabled)
	{
		if (selectionVisible)
		{
			changeSelection(QRect());
		}
		else
		{
			selectionVisible = true;
			changeSelection(image.rect());
		}
	}
}

void DisplaySurface::adjustSelection(int vTL, int hTL, int vBR, int hBR)
{
	if (selectionEnabled && selectionVisible)
	{
		QRect newSelection = selection.adjusted(vTL, hTL, vBR, hBR);
		if (imageRect.contains(newSelection))
		{
			changeSelection(newSelection);
		}
	}
}



DisplayCanvas::DisplayCanvas(const QPixmap &pixmap, DisplaySurface * surface, QGraphicsItem *parent) : QGraphicsPixmapItem(pixmap, parent)
{
	setAcceptHoverEvents(true);
	this->surface = surface;
	if (objCntr) qDebug() << "DisplayCanvas::objCntr != 0";
	validPaintPixmap = false;
	objCntr++;
}

DisplayCanvas::~DisplayCanvas()
{
	objCntr--;
}

void DisplayCanvas::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	int x = event->scenePos().x()*Globals::scalingFactor+0.4999999 + surface->getMousePositionCorrection().x();
	int y = event->scenePos().y()*Globals::scalingFactor+0.4999999 + surface->getMousePositionCorrection().y();
	QPoint pos = QPoint(x, y);
	surface->hoverAction(pos);
	QGraphicsItem::hoverEnterEvent(event);
}

void DisplayCanvas::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	int x = event->scenePos().x()*Globals::scalingFactor+0.4999999 + surface->getMousePositionCorrection().x();
	int y = event->scenePos().y()*Globals::scalingFactor+0.4999999 + surface->getMousePositionCorrection().y();
	QPoint pos = QPoint(x, y);
	surface->hoverLeaveAction(pos);
	QGraphicsItem::hoverLeaveEvent(event);
}

void DisplayCanvas::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
	int x = event->scenePos().x()*Globals::scalingFactor+0.4999999 + surface->getMousePositionCorrection().x();
	int y = event->scenePos().y()*Globals::scalingFactor+0.4999999 + surface->getMousePositionCorrection().y();
	QPoint pos = QPoint(x, y);
	surface->hoverAction(pos);
	QGraphicsItem::hoverMoveEvent(event);
}

void DisplayCanvas::setCanvasPixmap(const QPixmap &pixmap)
{
	if (validPaintPixmap)
	{
		validPaintPixmap = false;
		paintPixmap = QPixmap();
	}
	setPixmap(pixmap);
}

void DisplayCanvas::setZoom(double zoom)
{
	if (validPaintPixmap)
	{
		validPaintPixmap = false;
		paintPixmap = QPixmap();
	}
	this->zoom = zoom;
}

void DisplayCanvas::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	if (zoom < 1.0)
	{
		if (!validPaintPixmap)
		{
			if (Globals::prefs->getDisplayHigherQuality())
			{
				paintPixmap = pixmap().scaled( pixmap().size()*zoom, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
				validPaintPixmap = true;
			}
		}
	}
	else
	{
		validPaintPixmap = false;
	}
	if (validPaintPixmap)
	{
		painter->drawPixmap(boundingRect(), paintPixmap, paintPixmap.rect());
	}
	else
	{
		QGraphicsPixmapItem::paint(painter, option, widget);
	}
}

