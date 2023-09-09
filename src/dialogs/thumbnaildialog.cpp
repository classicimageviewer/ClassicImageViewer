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


#include "thumbnaildialog.h"
#include "globals.h"
#include <QDebug>
#include <QStandardItemModel>
#include <QScrollBar>
#include <QPainter>


ThumbnailDialog::ThumbnailDialog(QString indexedDirPath, QStringList indexedFiles, QWidget * parent) : QDialog(parent, Qt::Window)
{
	ui.setupUi(this);
	setModal(false);
	ui.listView->setGridSize(QSize(160,160));
	model = new ThumbnailModel(indexedDirPath, indexedFiles);
	ui.listView->setModel(model);
	scrollTimer.setSingleShot(true);
	connect(&scrollTimer, SIGNAL(timeout()), this, SLOT(scrollTimeout()));
	connect(ui.listView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(scrolled(int)));
	connect(ui.listView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(clicked(const QModelIndex &)));
	ui.listView->verticalScrollBar()->setSingleStep(Globals::prefs->getThumbnailsScrollSpeed());
}

ThumbnailDialog::~ThumbnailDialog()
{
	delete model;
}

void ThumbnailDialog::scrollTimeout()
{
	model->preload();
}

void ThumbnailDialog::scrolled(int i)
{
	Q_UNUSED(i);
	scrollTimer.start(50);
}

void ThumbnailDialog::clicked(const QModelIndex & index)
{
	if (index.isValid())
	{
		emit itemSelected(index.row());
	}
}

void ThumbnailDialog::selectItem(int index)
{
	ui.listView->setCurrentIndex(model->index(index));
}

void ThumbnailDialog::updateFileList(QString indexedDirPath, QStringList indexedFiles)
{
	ui.listView->setModel(NULL);
	delete model;
	model = new ThumbnailModel(indexedDirPath, indexedFiles);
	ui.listView->setModel(model);
}

ThumbnailModel::ThumbnailModel(QString indexedDirPath, QStringList indexedFiles)
{
	this->indexedDirPath = indexedDirPath;
	this->indexedFiles = indexedFiles;
	cacheSize = Globals::prefs->getThumbnailsCacheSize();
	if (cacheSize < 256) cacheSize = 256;
	if (cacheSize > 8192) cacheSize = 8192;
	pixmapCache = new QCache<int, QPixmap>(cacheSize);
	loaderCount = Globals::prefs->getThumbnailsThreads();
	if (loaderCount == 0)
	{
		loaderCount = QThread::idealThreadCount();
	}
	else
	{
		int exp = loaderCount - 1;
		loaderCount = 1;
		for (int i=0; i<exp; i++ )
		{
			loaderCount *= 2;
		}
	}
	
	if (loaderCount > 64) loaderCount = 64;
	if (loaderCount < 1) loaderCount = 1;
	requestedRowQueue = QList<int>();
	processedRowQueue = QList<int>();
	pathQueue = QStringList();
	for (int i=0; i<loaderCount; i++)
	{
		loaders[i] = new ThumbnailLoader(&mutex, &condWait, &requestedRowQueue, &processedRowQueue, &pathQueue, QSize(128,128));
		connect(loaders[i], SIGNAL(thumbnailLoaded(QPixmap*, int)), this, SLOT(thumbnailLoaded(QPixmap*, int)));
	}
	
	QImage img;
	img = QImage(":/pixmaps/pixmaps/loader.png");
	img.setDevicePixelRatio(Globals::scalingFactor);
	img = img.scaled(QSize(128,128)*Globals::scalingFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	placeholderThumbnail = QPixmap::fromImage(img);
	placeholderThumbnail.setDevicePixelRatio(Globals::scalingFactor);
	
	img = QImage(":/pixmaps/pixmaps/loadfailed.png");
	img.setDevicePixelRatio(Globals::scalingFactor);
	img = img.scaled(QSize(128,128)*Globals::scalingFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	unreadableThumbnail = QPixmap::fromImage(img);
	unreadableThumbnail.setDevicePixelRatio(Globals::scalingFactor);
	
	lastUsedRow = 0;
	connect(this, SIGNAL(loadRequest(const QString, const int)), this, SLOT(dispatch(const QString, const int)));
	connect(this, SIGNAL(changedLastUsedRow(const int)), this, SLOT(registerLastUsedRow(const int)));
}

ThumbnailModel::~ThumbnailModel()
{
	mutex.lock();
	for (int i=0; i<loaderCount; i++)
	{
		loaders[i]->setExitFlag();
	}
	mutex.unlock();
	condWait.wakeAll();
	for (int i=0; i<loaderCount; i++)
	{
		disconnect(loaders[i], SIGNAL(thumbnailLoaded(QPixmap*, int)), NULL, NULL);
		delete loaders[i];
	}
	delete pixmapCache;
}

void ThumbnailModel::preload()
{
	if (!Globals::prefs->getThumbnailsPreloading()) return;
	
	int amount = cacheSize/3;
	QList<int> preloadList;
	for (int i=0; i<amount; i++)
	{
		int lo = lastUsedRow - i;
		int hi = lastUsedRow + i;
		if (lo >= 0)
		{
			if (!pixmapCache->contains(lo))
			{
				preloadList.append(lo);
			}
		}
		if (hi < indexedFiles.length())
		{
			if (!pixmapCache->contains(hi))
			{
				preloadList.append(hi);
			}
		}
	}
	mutex.lock();
	while ((requestedRowQueue.length() > 0) && (requestedRowQueue.first() < 0))
	{
		requestedRowQueue.removeFirst();
		pathQueue.removeFirst();
	}
	for (const int row : preloadList)
	{
		if (!requestedRowQueue.contains(row) && !processedRowQueue.contains(row))
		{
			requestedRowQueue.prepend(-row);
			pathQueue.prepend(indexedDirPath + "/" + indexedFiles[row]);
		}
	}
	mutex.unlock();
	condWait.wakeAll();
}

int ThumbnailModel::rowCount(const QModelIndex & parent) const
{
	Q_UNUSED(parent);
	return indexedFiles.length();
}

QVariant ThumbnailModel::data(const QModelIndex & index, int role) const
{
	if (!index.isValid()) return QVariant();
	if (role == Qt::DecorationRole)
	{
		int row = index.row();
		if (pixmapCache->contains(row))
		{
			emit changedLastUsedRow(row);
			if (pixmapCache->object(row) == NULL)
			{
				return unreadableThumbnail;
			}
			return *pixmapCache->object(row);
		}
		else
		{
			if (indexedFiles.length() <= row) return QVariant();
			
			emit loadRequest(indexedDirPath + "/" + indexedFiles[row], row);
			
			return placeholderThumbnail;
		}
	} else
	if ((role == Qt::DisplayRole) || (role == Qt::ToolTipRole))
	{
		int row = index.row();
		if (indexedFiles.length() <= row) return QVariant();
		return indexedFiles[row];
	}
	return QVariant();
}

void ThumbnailModel::registerLastUsedRow(int row)
{
	lastUsedRow = row;
}

void ThumbnailModel::dispatch(const QString path, const int row)
{
	mutex.lock();
	if (requestedRowQueue.contains(row) || processedRowQueue.contains(row))
	{
		mutex.unlock();
		return;
	}
	else
	{
		int queueLimit = 512;
		if (queueLimit > cacheSize)
		{
			queueLimit = cacheSize;
		}
		while (requestedRowQueue.length() > queueLimit)
		{
			requestedRowQueue.removeFirst();	// delete oldest request
			pathQueue.removeFirst();
		}
		requestedRowQueue.append(row);
		pathQueue.append(path);
	}
	mutex.unlock();
	condWait.wakeOne();
}

void ThumbnailModel::thumbnailLoaded(QPixmap * pixmap, int row)
{
	pixmapCache->insert(row, pixmap);
	QModelIndex modelIndex = index(row);
	emit dataChanged(modelIndex, modelIndex);
	mutex.lock();
	processedRowQueue.removeAll(row);
	mutex.unlock();
}




ThumbnailLoader::ThumbnailLoader(QMutex * mutex, QWaitCondition * condWait, QList<int> * requestedRowQueue, QList<int> * processedRowQueue, QStringList * pathQueue, QSize thumbnailSize, QObject *parent) : QThread(parent)
{
	this->mutex = mutex;
	this->condWait = condWait;
	this->requestedRowQueue = requestedRowQueue;
	this->processedRowQueue = processedRowQueue;
	this->pathQueue = pathQueue;
	this->thumbnailSize = thumbnailSize*Globals::scalingFactor;
	exitFlag = false;
	imageIO = new ImageIO();
	start();
}

ThumbnailLoader::~ThumbnailLoader()
{
	wait();
	delete imageIO;
}

void ThumbnailLoader::setExitFlag()
{
	exitFlag = true;
}

void ThumbnailLoader::run()
{
	while (1)
	{
		mutex->lock();
		if (exitFlag)
		{
			mutex->unlock();
			return;
		}
		if (requestedRowQueue->isEmpty())
		{
			condWait->wait(mutex);
			mutex->unlock();
		}
		else
		{
			QString path = pathQueue->takeLast();	// LIFO is faster
			int row = requestedRowQueue->takeLast();
			if (row < 0) row = -row;	// preload
			processedRowQueue->append(row);
			mutex->unlock();
			QImage img = imageIO->loadThumbnail(path, thumbnailSize);
			if (img.isNull())
			{
				emit thumbnailLoaded(NULL, row);
			}
			else
			{
				img.setDevicePixelRatio(Globals::scalingFactor);
				QPixmap * pixmap = new QPixmap(thumbnailSize);
				pixmap->setDevicePixelRatio(Globals::scalingFactor);
				pixmap->fill(Qt::transparent);
				QPainter painter(pixmap);
				painter.drawImage((thumbnailSize.width() - img.width())/2, (thumbnailSize.height() - img.height())/2, img);
				painter.end();
				emit thumbnailLoaded(pixmap, row);
			}
			
		}
	}
}

