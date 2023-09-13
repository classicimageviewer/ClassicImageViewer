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


#ifndef BATCHDIALOG_H
#define BATCHDIALOG_H

#include "ui_batchdialog.h"
#include <QDialog>
#include <QTimer>
#include "io/imageIO.h"
#include "effects/effectHub.h"
#include <QThread>
#include <QMutex>

class BatchTypedefs
{
public:
	typedef struct {
		QString name;
		int effectId;
		QList<EffectBase::ParameterCluster> parameters;
	} EffectEntry;
	typedef struct {
		bool crop;
		int cropTop, cropRight, cropBottom, cropLeft;
		bool rote;
		int rotateDir, rotateCondition;
		bool hFlip, vFlip;
		bool autoColor;
		bool sharpen;
		bool grayscale, negative;
		bool resize;
		int resizeMode, resizeAlgorithm;
		double resizeW, resizeH;
		QList<EffectEntry> effectList;
		QString format;
		QList<IObase::ParameterCluster> formatParameterList;
		
	} ConversionParameters;
};

class BatchWorker : public QThread
{
	Q_OBJECT
private: // variables
	QMutex * mutex;
	int * listHead;
	QStringList * inputList;
	QStringList * outputList;
	QStringList * outputStatusList;
	BatchTypedefs::ConversionParameters * parameters;
	bool exitFlag;
	ImageIO * imageIO;
	EffectHub * effectHub;
public:
	BatchWorker(QMutex * mutex, int * listHead, QStringList * inputList, QStringList * outputList, QStringList * outputStatusList, BatchTypedefs::ConversionParameters * parameters, QObject *parent = NULL);
	~BatchWorker() override;
	void run() override;
	void setExitFlag();
};

class BatchDialog : public QDialog
{
	Q_OBJECT
private: // typedefs

private: // variables
	Ui_BatchDialog ui;
	bool batchProcessRunning;
	QString indexedDirPath;
	QStringList indexedFiles;
	ImageIO * imageIO;
	QStringList batchInput;
	QString previewPath;
	QTimer inputPreviewTimer;
	QList<BatchTypedefs::EffectEntry> effectList;
	QPixmap batchEmptyPixmap, batchSuccessPixmap, batchErrorPixmap;
	QMutex mutex;
	int listHead;
	QStringList outputList;
	QStringList outputStatusList;
	BatchTypedefs::ConversionParameters conversionParameters;
	int threadCount;
	BatchWorker * workers[64];
	QTimer progressTimer;
	int vipsRestore;
private: // functions
	void reject() override;
	void rebuildInputTable(int index);
	bool eventFilter(QObject* watched, QEvent* event);
	void loadPreview();
	void rebuildEffectTable(int index);
	void deleteOutputFormatParameters();
	void createOutputList();
	void collectParameters();
	void saveParameters();
	void rebuildProgressTable(int index);
	void startBatchProcessing();
	void abortBatchProcessing();
private slots:
	void pageChanged(int page);
	void closeButtonPressed(bool b);
	void prevPage(bool b);
	void nextPage(bool b);
	void inputAddFilesFromIndexedDir(bool b);
	void inputAddFiles(bool b);
	void inputRemoveFiles(bool b);
	void inputRemoveAll(bool b);
	void inputPreviewTimerTimeout();
	void resizeModeChanged(int index);
	void addEffect(bool b);
	void removeEffects(bool b);
	void newOutputDir(bool b);
	void outputFormatChanged(int index);
	void progressTimerTimeout();
public:
	BatchDialog(QString indexedDirPath, QStringList indexedFiles, ImageIO * imageIO, QWidget * parent = NULL);
	~BatchDialog();
};

#endif //BATCHDIALOG_H
