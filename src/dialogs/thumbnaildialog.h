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


#ifndef THUMBNAILDIALOG_H
#define THUMBNAILDIALOG_H

#include "ui_thumbnaildialog.h"
#include <QDialog>
#include <QAbstractListModel>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QCache>
#include <QTimer>

#include "io/imageIO.h"

class ThumbnailLoader : public QThread {
	Q_OBJECT
private: // variables
	QMutex * mutex;
	QWaitCondition * condWait;
	QList<int> * requestedRowQueue;
	QList<int> * processedRowQueue;
	QStringList * pathQueue;
	ImageIO * imageIO;
	QSize thumbnailSize;
	bool exitFlag;
public:
	ThumbnailLoader(QMutex * mutex, QWaitCondition * condWait, QList<int> * requestedRowQueue, QList<int> * processedRowQueue, QStringList * pathQueue, QSize thumbnailSize, QObject *parent = NULL);
	~ThumbnailLoader() override;
	void run() override;
	void setExitFlag();
signals:
	void thumbnailLoaded(QPixmap *pixmap, int row);
};




class ThumbnailModel : public QAbstractListModel {
	Q_OBJECT
private: // variables
	QMutex mutex;
	QWaitCondition condWait;
	QList<int> requestedRowQueue;
	QList<int> processedRowQueue;
	QStringList pathQueue;
	QString indexedDirPath;
	QStringList indexedFiles;
	int cacheSize;
	QCache<int, QPixmap> * pixmapCache;
	int loaderCount;
	ThumbnailLoader * loaders[64];
	QPixmap placeholderThumbnail, unreadableThumbnail;
	int lastUsedRow;
	QSize iconSize;
private: // functions
	void loadPlaceholders(void);
private slots:
	void dispatch(const QString path, const int row);
public:
	ThumbnailModel(QString indexedDirPath, QStringList indexedFiles, QSize iconSize);
	~ThumbnailModel();
	int rowCount(const QModelIndex & parent = QModelIndex()) const override;
	QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
	void preload();
public slots:
	void thumbnailLoaded(QPixmap *pixmap, int row);
	void registerLastUsedRow(int row);
	void setIconSize(QSize size);
signals:
	void loadRequest(const QString path, const int row) const;
	void changedLastUsedRow(int row) const;
};




class ThumbnailDialog : public QDialog
{
	Q_OBJECT
private: // variables
	Ui_ThumbnailDialog ui;
	ThumbnailModel * model;
	QTimer scrollTimer;
	int thumbnailSize;
private slots:
	void scrollTimeout();
	void scrolled(int i);
	void clicked(const QModelIndex & index);
	void thumbnailSizeChanged(int i);
private: // functions
	bool eventFilter(QObject* watched, QEvent* event);
	void setThumbnailSize(int size);
public:
	ThumbnailDialog(QString indexedDirPath, QStringList indexedFiles, QWidget * parent = NULL);
	~ThumbnailDialog();
	void selectItem(int index);
	void updateFileList(QString indexedDirPath, QStringList indexedFiles);
signals:
	void itemSelected(int index);

};

#endif //THUMBNAILDIALOG_H
