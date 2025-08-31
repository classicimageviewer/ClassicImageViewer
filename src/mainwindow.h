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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QLabel>
#include <QSpinBox>
#include <vector>
#include "ui_mainwindow.h"
#include "actions.h"
#include "widgets/displaywidget.h"
#include "widgets/savefiledialog.h"
#include "io/imageIO.h"
#include "dialogs/thumbnaildialog.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

private: // typedefs
	typedef struct {
		QAction * actionRef;
		Action event;
		int flags;
	} ActionLookUp_t;
	enum InternalState {UNLOADED=0, IMAGE_FROM_FILE, IMAGE_FROM_CLIPBOARD};
	enum SimpleFilter {NONE=0, ROT_R, ROT_L, MIRROR_V, MIRROR_H, GRAYS, NEGATIVE, COLORADJ, SHARPEN};

private: // variables
	QString fileToBeOpenedOnStartup;
	QTimer startupTimer;
	std::vector<ActionLookUp_t> ActionLookUpTable;
	InternalState internalState;
	QMenu * recentFilesMenu;
	QMenu * externalToolsMenu;
	QMenu * displayModeMenu;
	QMenu * zoomLevelMenu;
	int actionLock;
	ImageIO * imageIO;
	QSize currentImageSize;
	int clipboardCounter;
	QString currentImageName, currentFilePath, currentDirPath;
	int currentFileIndex;
	QString indexedDirPath;
	QStringList indexedFiles;
	DisplayWidget * display;
	QLabel * displayOverlayIndicator;
	double fullscreenZoomBefore;
	bool isFullscreen;
	QLineEdit * zoomDisplay;
	QSpinBox * indexDisplay;
	QLineEdit * dirCountDisplay;
	QAction * undoAction;
	QAction * redoAction;
	QList<QImage> undoHistory;
	int undoStackPosition;
	int undoIndex;
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
	bool moveToTrashWithGIO;
	bool moveToTrashWithKIO;
#endif
	int slideshowDirection;
	QTimer slideshowTimer;
	QLabel * statusBarResolution;
	QLabel * statusBarIndex;
	QLabel * statusBarZoom;
	QLabel * statusBarLastModified;
	QLabel * statusBarSelection;
	ThumbnailDialog * thumbnailDialog;
	QString mvCpTargetDir;
	QStringList shortCutInfo;
	bool blockSetImageSize;

private: // functions
	bool eventFilter(QObject* watched, QEvent* event);
	QAction * menuAddAction(QMenu * menu, QString text, Action event, const char *shortCut, int flags);
	void menuAddSeparator(QMenu * menu);
	void createMenu();
	QAction * searchQAction(Action a);
	void removeAction(Action a);
	void setupToolBar();
	void setupStatusBar();
	void setInternalState(InternalState newState);
	void applyInternalState();
	void addToClipboard(QImage image);
	void clearClipboard(bool forced = false);
	bool clipboardHasImage();
	QImage getFromClipboard();
	QStringList fastIndexer(QString dirPath, QStringList extensions, int ordering);
	void reIndexCurrentDir(bool forced = false);
	void loadCurrentFile();
	void doSimpleFilter(SimpleFilter f);
	void setFullscreen(bool fs);
	void setWindowSize();
	void setImageSize();
	void setImageAndWindowSize();
	void fitImageInto(QSize size);
	void resizeEvent(QResizeEvent *event) override;
	void updateDisplayOverlayIndicator();
	void clearUndoStack();
	void clearPastedUndoStack();
	void saveToUndoStack();
	void undoFromUndoStack();
	void redoFromUndoStack();
	void updateWindowTitle();
	void deleteThumbnailDialog();
	void showFileModificationTime(QString fullPath);
	void clearFileModificationTime();
	void addPathToRecentFiles(QString path);
	void updateRecentFilesMenu();
	void updateDisplayModeMenu();
	void setIndexDisplayNoSignals(int value, int maximum = 0);
	void closeEvent(QCloseEvent *event) override;
private slots:
	void sendAction(Action a);
	void actionSlot(Action a);
	void searchAction(QAction * action);
	void displayNeedNextImage();
	void displayNeedPrevImage();
	void displayNeedFirstImage();
	void displayNeedLastImage();
	void displayZoomChanged();
	void displaySelectionChanged();
	void displayPixelInfo();
	void startup();
	void indexDisplayChanged(int i);
	void slideshowTimeout();
	void thumbnailDialogClosed(int i);
	void thumbnailItemSelected(int index);
signals:
	void actionSignal(Action a);

public:
	MainWindow();
	~MainWindow();
	Ui_MainWindow ui;
	
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dragMoveEvent(QDragMoveEvent* event) override;
	void dragLeaveEvent(QDragLeaveEvent* event) override;
	void dropEvent(QDropEvent* event) override;

	void setFileToBeOpenedOnStartup(const QString arg);
	QStringList fileDialogOpen(const QString directoryPathOverride, bool multipleFiles = false);
	SaveFileDialog::ReturnCluster fileDialogSave(const QString directoryPathOverride, const QString defaultFileName, bool saveSelectedExtension = false);
	
	
public slots:

};

#endif //MAINWINDOW_H
