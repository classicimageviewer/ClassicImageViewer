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


#include "mainwindow.h"

#include <unistd.h>
#include "globals.h"
#include "menu.h"
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>

#include <QDebug>
#include <QWindow>
#include <QMessageBox>
#include <QFileInfo>
#include <QStandardPaths>
#include <QFileDialog>
#include <QClipboard>
#include <QMimeData>
#include <QKeyEvent>
#include <QScreen>
#include <QProgressDialog>
#include <QPrinter>
#include <QPrintDialog>
#include <QDateTime>
#include <QScrollBar>
#include <QProcess>
#include <QCollator>
#include <QElapsedTimer>

#include "dialogs/preferencesdialog.h"
#include "dialogs/resizedialog.h"
#include "dialogs/infodialog.h"
#include "dialogs/slideshowdialog.h"
#include "dialogs/rotatedialog.h"
#include "dialogs/addborderdialog.h"
#include "dialogs/padtosizedialog.h"
#include "dialogs/coloradjdialog.h"
#include "dialogs/licensedialog.h"
#include "dialogs/aboutdialog.h"
#include "dialogs/renamedialog.h"
#include "dialogs/targetdirdialog.h"
#include "dialogs/effectsdialog.h"
#include "dialogs/shortcutsdialog.h"
#include "dialogs/pastetosidedialog.h"
#include "dialogs/customselectiondialog.h"
#include "dialogs/batchdialog.h"
#include "dialogs/exttoolconfigdialog.h"

#include "modules/resizer.h"
#include "modules/autocolor.h"
#include "modules/sharpener.h"


MainWindow::MainWindow() : QMainWindow()
{
	ui.setupUi(this);
	
	QApplication::setStyle("fusion");
	
	fileToBeOpenedOnStartup = QString();
	actionLock = 0;
	ActionLookUpTable = std::vector<ActionLookUp_t>();
	currentImageName = QString();
	currentFilePath = QString();
	currentDirPath = QString();
	indexedDirPath = QString();
	indexedFiles = QStringList();
	isFullscreen = false;
	display = ui.displayWidget;
	display->setBackgroundShade(Globals::prefs->getDisplayBackground());
	displayOverlayIndicator = new QLabel();
	QVBoxLayout* layout = new QVBoxLayout(display);
	layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	layout->addWidget(displayOverlayIndicator);
	clipboardCounter = 0;
	thumbnailDialog = NULL;
	mvCpTargetDir = QString();
	shortCutInfo = QStringList();
	blockSetImageSize = false;

	imageIO = new ImageIO();
	
	createMenu();
	setupToolBar();
	setupStatusBar();
	ui.menuBar->setVisible(Globals::prefs->getMenubarVisible());
	ui.toolBar->setVisible(Globals::prefs->getToolbarVisible());
	ui.statusBar->setVisible(Globals::prefs->getStatusbarVisible());
	ui.toolBar->setSizePolicy((Globals::prefs->getEnableToolbarShrinking() ? (QSizePolicy::Expanding) : (QSizePolicy::MinimumExpanding)), QSizePolicy::Minimum);
	
	connect(&startupTimer, SIGNAL(timeout()), this, SLOT(startup()));
	startupTimer.setSingleShot(true);
	startupTimer.start(66);
	slideshowTimer.stop();
	slideshowDirection = 0;
	
	connect(this, SIGNAL(actionSignal(Action)), this, SLOT(actionSlot(Action)));
	connect(display, SIGNAL(needNextImage()), this, SLOT(displayNeedNextImage()));
	connect(display, SIGNAL(needPrevImage()), this, SLOT(displayNeedPrevImage()));
	connect(display, SIGNAL(needFirstImage()), this, SLOT(displayNeedFirstImage()));
	connect(display, SIGNAL(needLastImage()), this, SLOT(displayNeedLastImage()));
	connect(display, SIGNAL(zoomChanged()), this, SLOT(displayZoomChanged()));
	connect(display, SIGNAL(selectionChanged()), this, SLOT(displaySelectionChanged()));
	connect(display, SIGNAL(pixelInfo()), this, SLOT(displayPixelInfo()));
	connect(&slideshowTimer, SIGNAL(timeout()), this, SLOT(slideshowTimeout()));
	this->installEventFilter(this);
	setAcceptDrops(true);
	
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
	// no QFile::moveToTrash() method -> try external software
	moveToTrashWithGIO = false;
	moveToTrashWithKIO = false;
	
	QProcess testTrashProc;
	testTrashProc.setStandardErrorFile(QProcess::nullDevice());
	testTrashProc.setStandardOutputFile(QProcess::nullDevice());
	
	testTrashProc.start("gio", {"help"});
	moveToTrashWithGIO = (!(!testTrashProc.waitForFinished(100) || (testTrashProc.error() == QProcess::FailedToStart)));
	
	testTrashProc.start("kioclient5", {"--help"});
	moveToTrashWithKIO = (!(!testTrashProc.waitForFinished(100) || (testTrashProc.error() == QProcess::FailedToStart)));
#endif
	clearUndoStack();
	if (Globals::prefs->getMaximizedWindow())
	{
		setWindowState((windowState() & ~Qt::WindowActive) | Qt::WindowMaximized);
	}
	else
	{
		this->move(Globals::prefs->getWindowPosition());
	}
	setInternalState(UNLOADED);
}

MainWindow::~MainWindow()
{
	deleteThumbnailDialog();
	delete imageIO;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (thumbnailDialog)
	{
		deleteThumbnailDialog();
	}
	if (Globals::prefs->getClearClipboardOnExit())
	{
		clearClipboard();
	}
	Globals::prefs->setWindowPosition(this->pos());
	QMainWindow::closeEvent(event);
}

void MainWindow::setFileToBeOpenedOnStartup(const QString arg)
{
	fileToBeOpenedOnStartup = arg;
}

void MainWindow::startup()
{
	if (!fileToBeOpenedOnStartup.isEmpty())
	{
		QFileInfo file(fileToBeOpenedOnStartup);
		if (file.exists())
		{
			currentFilePath = file.fileName();
			currentDirPath = file.absolutePath();
			addPathToRecentFiles(file.absoluteFilePath());
			sendAction(ACT_REOPEN);
			if (Globals::prefs->getStartFullscreen())
			{
				sendAction(ACT_TOGGLE_FULLSCREEN);
			}
		}
		fileToBeOpenedOnStartup = QString();
	}
}


bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
	QKeyEvent * keyEvent = (QKeyEvent*)event;
	if (watched == indexDisplay)
	{
		// allow editing keys (Backspace, etc.)
		if (event->type() == QEvent::ShortcutOverride)
		{
			event->accept();
			return true;
		}
		// catch those nasty Key_Return triggering ACT_TOGGLE_FULLSCREEN on spinbox user entry
		if (event->type() == QEvent::KeyPress)
		{
			if (keyEvent->key() == Qt::Key_Return)
			{
				event->accept();
				indexDisplay->interpretText();
				return true;
			}
			if (keyEvent->key() == Qt::Key_Escape)	// lose focus
			{
				event->accept();
				setIndexDisplayNoSignals(indexDisplay->value());
				indexDisplay->clearFocus();
				display->setFocus();
				return true;
			}
		}
		return false;
	}
	if (watched == zoomDisplay)
	{
		if (event->type() == QEvent::MouseButtonPress)
		{
			if (internalState != UNLOADED)
			{
				QMenu *menu = new QMenu(this);
				menu->addAction(searchQAction(ACT_ZOOM_10));
				menu->addAction(searchQAction(ACT_ZOOM_15));
				menu->addAction(searchQAction(ACT_ZOOM_20));
				menu->addAction(searchQAction(ACT_ZOOM_25));
				menu->addAction(searchQAction(ACT_ZOOM_33));
				menu->addAction(searchQAction(ACT_ZOOM_50));
				menu->addAction(searchQAction(ACT_ZOOM_66));
				menu->addAction(searchQAction(ACT_ZOOM_100));
				menu->addAction(searchQAction(ACT_ZOOM_125));
				menu->addAction(searchQAction(ACT_ZOOM_150));
				menu->addAction(searchQAction(ACT_ZOOM_175));
				menu->addAction(searchQAction(ACT_ZOOM_200));
				menu->addAction(searchQAction(ACT_ZOOM_300));
				menu->addAction(searchQAction(ACT_ZOOM_400));
				connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(searchAction(QAction*)));
				menu->popup(zoomDisplay->mapToGlobal(QPoint(0,0+zoomDisplay->height())));
				return true;
			}
		}
		return false;
	}
	/*if (event->type() == QEvent::KeyPress) <-- not needed anymore; QActions are added to the display widget
	{
		if (keyEvent->key() == Qt::Key_Return) sendAction(ACT_TOGGLE_FULLSCREEN);
		if (keyEvent->key() == Qt::Key_Escape) sendAction(ACT_EXIT);
	}*/
	return QObject::eventFilter(watched, event);
}

QAction * MainWindow::menuAddAction(QMenu * menu, QString text, Action event, const char *shortCut, int flags)
{
	QAction * a = menu->addAction(text);
	if (shortCut)
	{
		QString sc = QString(shortCut);
		QStringList shortCutList = sc.split(";");
		QList<QKeySequence> shortCuts = QList<QKeySequence>();
		for (const QString & s : shortCutList)
		{
			if (s.length() > 0)
			{
				shortCuts.append(QKeySequence(s));
				shortCutInfo.append(QKeySequence(s).toString() + "&" + text.remove('&'));
			}
		}
		if (shortCuts.length() > 0)
		{
			a->setShortcuts(shortCuts);
		}
	}
	ActionLookUp_t e;
	e.actionRef = a;
	e.event = event;
	e.flags = flags;
	ActionLookUpTable.push_back(e);
	display->addAction(a);	// make shortcuts available in fullscreen mode
	return a;
}

void MainWindow::menuAddSeparator(QMenu * menu)
{
	menu->addSeparator();
}

void MainWindow::createMenu()
{
	menuAddAction(ui.menuFile, tr("&Open"), ACT_OPEN, "O",  ACTDISABLE_FULLSCREEN);
	menuAddAction(ui.menuFile, tr("&Reload"), ACT_REOPEN, "Shift+R",  ACTDISABLE_UNLOADED | ACTDISABLE_CLIPBOARD);
	menuAddAction(ui.menuFile, tr("Reopen in new app"), ACT_OPEN_IN_NEW_APP, "Ctrl+N",  ACTDISABLE_UNLOADED | ACTDISABLE_CLIPBOARD);
	recentFilesMenu = new QMenu(tr("Recent &files"));
	ui.menuFile->addMenu(recentFilesMenu);
	updateRecentFilesMenu();
	menuAddSeparator(ui.menuFile);
	menuAddAction(ui.menuFile, tr("&Thumbnails"), ACT_THUMBNAILS, "T",  ACTDISABLE_UNLOADED);
	menuAddAction(ui.menuFile, tr("Slidesho&w"), ACT_SLIDESHOW, "W",  ACTDISABLE_UNLOADED | ACTDISABLE_FULLSCREEN);
	menuAddAction(ui.menuFile, tr("&Batch conversion"), ACT_BATCH, "B",  ACTDISABLE_FULLSCREEN);
	menuAddSeparator(ui.menuFile);
	menuAddAction(ui.menuFile, tr("Re&name file"), ACT_FILE_RENAME, "F2",  ACTDISABLE_UNLOADED | ACTDISABLE_FULLSCREEN);
	menuAddAction(ui.menuFile, tr("Select target directory"), ACT_SELECT_TARGET_DIR, NULL, 0);
	menuAddAction(ui.menuFile, tr("&Move file"), ACT_FILE_MOVE, "F7",  ACTDISABLE_UNLOADED | ACTDISABLE_CLIPBOARD);
	menuAddAction(ui.menuFile, tr("&Copy file"), ACT_FILE_COPY, "F8",  ACTDISABLE_UNLOADED | ACTDISABLE_CLIPBOARD);
	menuAddAction(ui.menuFile, tr("&Delete file"), ACT_FILE_DELETE, "Del",  ACTDISABLE_UNLOADED | ACTDISABLE_CLIPBOARD);
	menuAddSeparator(ui.menuFile);
	menuAddAction(ui.menuFile, tr("Sa&ve"), ACT_SAVE, "Ctrl+S",  ACTDISABLE_UNLOADED);
	menuAddAction(ui.menuFile, tr("&Save as"), ACT_SAVE_AS, "S",  ACTDISABLE_UNLOADED);
	menuAddAction(ui.menuFile, tr("Save selection as"), ACT_SAVE_SELECTION_AS, "Ctrl+Shift+S",  ACTDISABLE_UNLOADED);
	menuAddAction(ui.menuFile, tr("&Print"), ACT_PRINT, "Ctrl+P",  ACTDISABLE_UNLOADED);
	menuAddSeparator(ui.menuFile);
	menuAddAction(ui.menuFile, tr("E&xit"), ACT_EXIT, "Esc",  0);
	connect(ui.menuFile, SIGNAL(triggered(QAction*)), this, SLOT(searchAction(QAction*)));

	undoAction = menuAddAction(ui.menuEdit, tr("&Undo"), ACT_UNDO, "Ctrl+Z",  0);
	redoAction = menuAddAction(ui.menuEdit, tr("R&edo"), ACT_REDO, "Ctrl+J",  0);
	menuAddSeparator(ui.menuEdit);
	menuAddAction(ui.menuEdit, tr("Cust&om selection"), ACT_CUSTOM_SELECTION, "Shift+C",  ACTDISABLE_UNLOADED | ACTDISABLE_FULLSCREEN);
	menuAddAction(ui.menuEdit, tr("Cut &selection"), ACT_CUT_SELECTION, "Ctrl+X",  ACTDISABLE_UNLOADED);
	menuAddAction(ui.menuEdit, tr("C&rop selection"), ACT_CROP_SELECTION, "Ctrl+Y",  ACTDISABLE_UNLOADED);
	menuAddSeparator(ui.menuEdit);
	menuAddAction(ui.menuEdit, tr("&Copy"), ACT_COPY, "Ctrl+C",  ACTDISABLE_UNLOADED);
	menuAddAction(ui.menuEdit, tr("&Paste"), ACT_PASTE, "Ctrl+V",  0);
	menuAddAction(ui.menuEdit, tr("Paste to side"), ACT_PASTE_TO_SIDE, "Ctrl+D",  ACTDISABLE_UNLOADED);
	menuAddAction(ui.menuEdit, tr("Unloa&d"), ACT_UNLOAD, "D",  ACTDISABLE_UNLOADED);
	menuAddSeparator(ui.menuEdit);
	menuAddAction(ui.menuEdit, tr("C&lear clipboard"), ACT_CLEAR_CLIPBOARD, NULL,  0);
	connect(ui.menuEdit, SIGNAL(triggered(QAction*)), this, SLOT(searchAction(QAction*)));
	
	menuAddAction(ui.menuImage, tr("&Information"), ACT_INFO, "I",  ACTDISABLE_UNLOADED | ACTDISABLE_CLIPBOARD);
	menuAddSeparator(ui.menuImage);
	menuAddAction(ui.menuImage, tr("Rotate &left"), ACT_ROTATE_L, "L",  ACTDISABLE_UNLOADED);
	menuAddAction(ui.menuImage, tr("Rotate &right"), ACT_ROTATE_R, "R",  ACTDISABLE_UNLOADED);
	menuAddAction(ui.menuImage, tr("C&ustom rotation"), ACT_ROTATE_C, "Ctrl+U",  ACTDISABLE_UNLOADED | ACTDISABLE_FULLSCREEN);
	menuAddAction(ui.menuImage, tr("&Vertical flip"), ACT_FLIP_V, "V",  ACTDISABLE_UNLOADED);
	menuAddAction(ui.menuImage, tr("&Horizontal flip"), ACT_FLIP_H, "H",  ACTDISABLE_UNLOADED);
	menuAddSeparator(ui.menuImage);
	menuAddAction(ui.menuImage, tr("Re&size"), ACT_RESIZE, "Ctrl+R",  ACTDISABLE_UNLOADED | ACTDISABLE_FULLSCREEN);
	menuAddAction(ui.menuImage, tr("Add &border"), ACT_ADD_BORDER, NULL,  ACTDISABLE_UNLOADED | ACTDISABLE_FULLSCREEN);
	menuAddAction(ui.menuImage, tr("&Pad to size"), ACT_PAD_TO_SIZE, NULL,  ACTDISABLE_UNLOADED | ACTDISABLE_FULLSCREEN);
	menuAddSeparator(ui.menuImage);
	menuAddAction(ui.menuImage, tr("I&ncrease color depth"), ACT_COLOR_DEPTH_INC, NULL,  ACTDISABLE_UNLOADED | ACTDISABLE_FULLSCREEN);
	menuAddAction(ui.menuImage, tr("&Decrease color depth"), ACT_COLOR_DEPTH_DEC, NULL,  ACTDISABLE_UNLOADED | ACTDISABLE_FULLSCREEN);
	menuAddSeparator(ui.menuImage);
	menuAddAction(ui.menuImage, tr("&Grayscale"), ACT_GRAYSCALE, "Ctrl+G",  ACTDISABLE_UNLOADED);
	menuAddAction(ui.menuImage, tr("Nega&tive"), ACT_NEGATIVE, "Ctrl+Shift+N",  ACTDISABLE_UNLOADED);
	menuAddAction(ui.menuImage, tr("Adjust &colors"), ACT_COLOR_ADJUST, "Shift+G",  ACTDISABLE_UNLOADED | ACTDISABLE_FULLSCREEN);
	menuAddSeparator(ui.menuImage);
	menuAddAction(ui.menuImage, tr("Auto c&olor adjust"), ACT_AUTO_COLOR, "Shift+U",  ACTDISABLE_UNLOADED);
	menuAddAction(ui.menuImage, tr("Sh&arpen"), ACT_SHARPEN, "Shift+S",  ACTDISABLE_UNLOADED);
	menuAddAction(ui.menuImage, tr("&Effects"), ACT_EFFECTS, "Ctrl+E",  ACTDISABLE_UNLOADED | ACTDISABLE_FULLSCREEN);
	menuAddSeparator(ui.menuImage);
	externalToolsMenu = ui.menuImage->addMenu(tr("E&xternal tools"));
	menuAddAction(externalToolsMenu, tr("Configure"), ACT_EXTERNAL_TOOL_CONFIG, NULL,  0);
	menuAddSeparator(externalToolsMenu);
	menuAddAction(externalToolsMenu, tr("External tool 1"), ACT_EXTERNAL_TOOL_1, "Alt+1",  ACTDISABLE_UNLOADED | ACTDISABLE_FULLSCREEN);
	menuAddAction(externalToolsMenu, tr("External tool 2"), ACT_EXTERNAL_TOOL_2, "Alt+2",  ACTDISABLE_UNLOADED | ACTDISABLE_FULLSCREEN);
	menuAddAction(externalToolsMenu, tr("External tool 3"), ACT_EXTERNAL_TOOL_3, "Alt+3",  ACTDISABLE_UNLOADED | ACTDISABLE_FULLSCREEN);
	menuAddAction(externalToolsMenu, tr("External tool 4"), ACT_EXTERNAL_TOOL_4, "Alt+4",  ACTDISABLE_UNLOADED | ACTDISABLE_FULLSCREEN);
	menuAddAction(externalToolsMenu, tr("External tool 5"), ACT_EXTERNAL_TOOL_5, "Alt+5",  ACTDISABLE_UNLOADED | ACTDISABLE_FULLSCREEN);
	menuAddAction(externalToolsMenu, tr("External tool 6"), ACT_EXTERNAL_TOOL_6, "Alt+6",  ACTDISABLE_UNLOADED | ACTDISABLE_FULLSCREEN);
	menuAddAction(externalToolsMenu, tr("External tool 7"), ACT_EXTERNAL_TOOL_7, "Alt+7",  ACTDISABLE_UNLOADED | ACTDISABLE_FULLSCREEN);
	menuAddAction(externalToolsMenu, tr("External tool 8"), ACT_EXTERNAL_TOOL_8, "Alt+8",  ACTDISABLE_UNLOADED | ACTDISABLE_FULLSCREEN);
	menuAddAction(externalToolsMenu, tr("External tool 9"), ACT_EXTERNAL_TOOL_9, "Alt+9",  ACTDISABLE_UNLOADED | ACTDISABLE_FULLSCREEN);
	connect(ui.menuImage, SIGNAL(triggered(QAction*)), this, SLOT(searchAction(QAction*)));
	
	menuAddAction(ui.menuOptions, tr("&Properties"), ACT_SETTINGS, "P",  0);
	menuAddAction(ui.menuOptions, tr("&Minimize"), ACT_MINIMIZE, "M",  0);
	connect(ui.menuOptions, SIGNAL(triggered(QAction*)), this, SLOT(searchAction(QAction*)));
	
	menuAddAction(ui.menuView, tr("Toggle &status bar"), ACT_TOGGLE_STATUSBAR, "Alt+Shift+S",  ACTDISABLE_FULLSCREEN);
	menuAddAction(ui.menuView, tr("Toggle &toolbar"), ACT_TOGGLE_TOOLBAR, "Alt+Shift+T",  ACTDISABLE_FULLSCREEN);
	menuAddAction(ui.menuView, tr("Toggle &menu bar"), ACT_TOGGLE_MENUBAR, "Alt+Shift+M",  ACTDISABLE_FULLSCREEN);
	menuAddSeparator(ui.menuView);
	displayModeMenu = ui.menuView->addMenu(tr("&Display mode"));
	menuAddAction(displayModeMenu, tr("Fit window to images"), ACT_DISPLAY_MODE_0, "Shift+O", ACTDISABLE_FULLSCREEN);
	menuAddAction(displayModeMenu, tr("Fit all images to window"), ACT_DISPLAY_MODE_1, "Shift+W", ACTDISABLE_FULLSCREEN);
	menuAddAction(displayModeMenu, tr("Fit large images to window"), ACT_DISPLAY_MODE_2, NULL, ACTDISABLE_FULLSCREEN);
	menuAddAction(displayModeMenu, tr("Fit all images to desktop"), ACT_DISPLAY_MODE_4, "f", ACTDISABLE_FULLSCREEN);
	menuAddAction(displayModeMenu, tr("Fit large images to desktop"), ACT_DISPLAY_MODE_5, "Shift+F", ACTDISABLE_FULLSCREEN);
	menuAddAction(displayModeMenu, tr("Do not fit"), ACT_DISPLAY_MODE_3, NULL, ACTDISABLE_FULLSCREEN);
	updateDisplayModeMenu();
	menuAddSeparator(ui.menuView);
	menuAddAction(ui.menuView, tr("&Full screen"), ACT_TOGGLE_FULLSCREEN, "Return",  ACTDISABLE_UNLOADED);
	menuAddSeparator(ui.menuView);
	menuAddAction(ui.menuView, tr("&Next file in directory"), ACT_NEXT_FILE, "Space",  ACTDISABLE_UNLOADED);
	menuAddAction(ui.menuView, tr("&Previous file in directory"), ACT_PREV_FILE, "Backspace",  ACTDISABLE_UNLOADED);
	menuAddAction(ui.menuView, tr("Fi&rst file in directory"), ACT_FIRST_FILE, "Ctrl+Home",  ACTDISABLE_UNLOADED);
	menuAddAction(ui.menuView, tr("&Last file in directory"), ACT_LAST_FILE, "Ctrl+End",  ACTDISABLE_UNLOADED);
	menuAddSeparator(ui.menuView);
	menuAddAction(ui.menuView, tr("Zoom &in"), ACT_ZOOM_IN, "+",  ACTDISABLE_UNLOADED);
	menuAddAction(ui.menuView, tr("Zoom &out"), ACT_ZOOM_OUT, "-",  ACTDISABLE_UNLOADED);
	menuAddAction(ui.menuView, tr("Zoom 1:1"), ACT_ZOOM_1, "Ctrl+H",  ACTDISABLE_UNLOADED);
	zoomLevelMenu = new QMenu(tr("&Zoom levels"));
	ui.menuView->addMenu(zoomLevelMenu);
	menuAddAction(zoomLevelMenu, tr("10%"), ACT_ZOOM_10, NULL,  ACTDISABLE_UNLOADED);
	menuAddAction(zoomLevelMenu, tr("15%"), ACT_ZOOM_15, NULL,  ACTDISABLE_UNLOADED);
	menuAddAction(zoomLevelMenu, tr("20%"), ACT_ZOOM_20, NULL,  ACTDISABLE_UNLOADED);
	menuAddAction(zoomLevelMenu, tr("25%"), ACT_ZOOM_25, NULL,  ACTDISABLE_UNLOADED);
	menuAddAction(zoomLevelMenu, tr("33%"), ACT_ZOOM_33, NULL,  ACTDISABLE_UNLOADED);
	menuAddAction(zoomLevelMenu, tr("50%"), ACT_ZOOM_50, NULL,  ACTDISABLE_UNLOADED);
	menuAddAction(zoomLevelMenu, tr("66%"), ACT_ZOOM_66, NULL,  ACTDISABLE_UNLOADED);
	menuAddAction(zoomLevelMenu, tr("100%"), ACT_ZOOM_100, NULL,  ACTDISABLE_UNLOADED);
	menuAddAction(zoomLevelMenu, tr("125%"), ACT_ZOOM_125, NULL,  ACTDISABLE_UNLOADED);
	menuAddAction(zoomLevelMenu, tr("150%"), ACT_ZOOM_150, NULL,  ACTDISABLE_UNLOADED);
	menuAddAction(zoomLevelMenu, tr("175%"), ACT_ZOOM_175, NULL,  ACTDISABLE_UNLOADED);
	menuAddAction(zoomLevelMenu, tr("200%"), ACT_ZOOM_200, NULL,  ACTDISABLE_UNLOADED);
	menuAddAction(zoomLevelMenu, tr("300%"), ACT_ZOOM_300, NULL,  ACTDISABLE_UNLOADED);
	menuAddAction(zoomLevelMenu, tr("400%"), ACT_ZOOM_400, NULL,  ACTDISABLE_UNLOADED);
	connect(ui.menuView, SIGNAL(triggered(QAction*)), this, SLOT(searchAction(QAction*)));
	
	menuAddAction(ui.menuHelp, tr("&License"), ACT_LICENSE, NULL,  0);
	menuAddAction(ui.menuHelp, tr("&Shortcuts"), ACT_SHORTCUTS, NULL,  0);
	menuAddAction(ui.menuHelp, tr("&About"), ACT_ABOUT, NULL,  0);
	connect(ui.menuHelp, SIGNAL(triggered(QAction*)), this, SLOT(searchAction(QAction*)));
	
	//"hidden" shortucts (handled in DisplayWidget)
	shortCutInfo.append(QKeySequence("Home").toString() + "&" + tr("First file in directory"));
	shortCutInfo.append(QKeySequence("End").toString() + "&" + tr("Last file in directory"));
	shortCutInfo.append(QKeySequence("Left").toString() + "&" + tr("Previous file in directory"));
	shortCutInfo.append(QKeySequence("Right").toString() + "&" + tr("Next file in directory"));
	shortCutInfo.append(QKeySequence("Up").toString() + "&" + tr("Previous file in directory"));
	shortCutInfo.append(QKeySequence("Down").toString() + "&" + tr("Next file in directory"));
	shortCutInfo.append(QKeySequence("PgUp").toString() + "&" + tr("Previous file in directory"));
	shortCutInfo.append(QKeySequence("PgDown").toString() + "&" + tr("Next file in directory"));
	shortCutInfo.append(QKeySequence("Ctrl+A").toString() + "&" + tr("Toggle selection all"));
	shortCutInfo.append(QKeySequence("Ctrl+Left,Ctrl+Right").toString() + "&" + tr("Adjust left side of the selection"));
	shortCutInfo.append(QKeySequence("Ctrl+Up,Ctrl+Down").toString() + "&" + tr("Adjust top side of the selection"));
	shortCutInfo.append(QKeySequence("Alt+Left,Alt+Right").toString() + "&" + tr("Adjust right side of the selection"));
	shortCutInfo.append(QKeySequence("Alt+Up,Alt+Down").toString() + "&" + tr("Adjust bottom side of the selection"));
	shortCutInfo.append(QKeySequence("Shift+Left,Shift+Right,Shift+Up,Shift+Down").toString() + "&" + tr("Move selection"));
}

void MainWindow::setInternalState(InternalState newState)
{
	internalState = newState;
	applyInternalState();
}

void MainWindow::applyInternalState()
{
	for (const ActionLookUp_t & element : ActionLookUpTable)
	{
		bool enabled = true;
		if ((element.flags & ACTDISABLE_FULLSCREEN) && (isFullscreen)) enabled = false;
		if ((element.flags & ACTDISABLE_UNLOADED) && (internalState == UNLOADED)) enabled = false;
		if ((element.flags & ACTDISABLE_CLIPBOARD) && (internalState == IMAGE_FROM_CLIPBOARD)) enabled = false;
		element.actionRef->setEnabled(enabled);
	}
}

void MainWindow::sendAction(Action a)
{
	// prevent user input congestion
	if (actionLock > 0)
	{
		if ((a > ACTGROUP_NAVIGATION_BEGIN) && (a < ACTGROUP_NAVIGATION_END)) return;
		if ((a > ACTGROUP_FILTER_BEGIN) && (a < ACTGROUP_FILTER_END)) return;
	}
	emit actionSignal(a);
}

void MainWindow::actionSlot(Action a)
{
	actionLock++;
	switch(a)
	{
		case ACT_OPEN:
			{
				QStringList list = fileDialogOpen(QString());
				if (!list.isEmpty())
				{
					QString fileName = list.at(0);
					if (!fileName.isEmpty())
					{
						QFileInfo file(fileName);
						if (file.exists())
						{
							Globals::prefs->setLastOpenedDir(file.absolutePath());
							currentFilePath = file.fileName();
							currentDirPath = file.absolutePath();
							loadCurrentFile();
							reIndexCurrentDir();
							
							addPathToRecentFiles(file.absoluteFilePath());
						}
					}
				}
			}
			break;
		case ACT_REOPEN:
			{
				if (internalState != IMAGE_FROM_CLIPBOARD)
				{
					loadCurrentFile();
					reIndexCurrentDir(true);
				}
			}
			break;
		case ACT_OPEN_IN_NEW_APP:
			{
				QProcess newProcess;
				QStringList arguments;
				arguments.append(currentDirPath + "/" + currentFilePath);
				newProcess.setProgram(QCoreApplication::arguments().at(0));
				newProcess.setArguments(arguments);
				newProcess.setStandardErrorFile(QProcess::nullDevice());
				newProcess.setStandardOutputFile(QProcess::nullDevice());
				newProcess.startDetached();
			}
			break;
		case ACT_RECENT_FILE_0:
		case ACT_RECENT_FILE_1:
		case ACT_RECENT_FILE_2:
		case ACT_RECENT_FILE_3:
		case ACT_RECENT_FILE_4:
		case ACT_RECENT_FILE_5:
		case ACT_RECENT_FILE_6:
		case ACT_RECENT_FILE_7:
			{
				int index = (a - ACT_RECENT_FILE_0);
				QStringList list = Globals::prefs->getRecentFiles();
				if (index < list.length())
				{
					QFileInfo file(list.at(index));
					if (file.exists())
					{
						currentFilePath = file.fileName();
						currentDirPath = file.absolutePath();
						sendAction(ACT_REOPEN);
					}
					else
					{
						list.removeAt(index);
						Globals::prefs->setRecentFiles(list);
						updateRecentFilesMenu();
					}
				}
			}
			break;
		case ACT_CLEAR_RECENT_FILES:
			Globals::prefs->setRecentFiles(QStringList());
			updateRecentFilesMenu();
			break;
		case ACT_THUMBNAILS:
			{
				if (thumbnailDialog == NULL)
				{
					QApplication::setOverrideCursor(Qt::WaitCursor);
					thumbnailDialog = new ThumbnailDialog(indexedDirPath, indexedFiles);
					thumbnailDialog->show();
					thumbnailDialog->selectItem(indexedFiles.indexOf(currentFilePath));
					connect(thumbnailDialog, SIGNAL(itemSelected(int)), this, SLOT(thumbnailItemSelected(int)));
					connect(thumbnailDialog, SIGNAL(finished(int)), this, SLOT(thumbnailDialogClosed(int)));
					QApplication::restoreOverrideCursor();
				}
				else
				{
					thumbnailDialog->activateWindow();
				}
			}
			break;
		case ACT_SLIDESHOW:
			{
				SlideshowDialog * d = new SlideshowDialog();
				if (d->exec() == QDialog::Accepted)
				{
					setFullscreen(true);
					int interval = d->getIntervalMs();
					slideshowDirection = ((interval < 0) ? -1:1);
					if (interval < 0) interval *= -1;
					if (interval < 10) interval = 10;
					slideshowTimer.start(interval);
					d->savePreferences();
				}
				delete d;
			}
			break;
		case ACT_BATCH:
			{
				BatchDialog * d = new BatchDialog(indexedDirPath, indexedFiles, imageIO);
				d->exec();
				delete d;
			}
			break;
		case ACT_FILE_RENAME:
			{
				RenameDialog * d = new RenameDialog(currentFilePath);
				if (d->exec() == QDialog::Accepted)
				{
					QString newName = d->getNewName();
					if (newName.length() < 1)
					{
						QMessageBox::critical(this, tr("Error"), tr("Cannot rename to empty filename."));
					}
					else
					{
						QFile file(currentDirPath + "/" + currentFilePath);
						if (file.rename(currentDirPath + "/" + newName))
						{
							int index = indexedFiles.indexOf(currentFilePath);
							currentImageName = newName;
							indexedFiles.replace(index, newName);
							currentFilePath = newName;
							updateWindowTitle();
						}
						else
						{
							QMessageBox::critical(this, tr("Error"), tr("Failed to rename the file.\nMaybe the provided file name already exists."));
						}
					}
				}
				delete d;
			}
			break;
		case ACT_SELECT_TARGET_DIR:
			{
				TargetDirDialog * d = new TargetDirDialog(mvCpTargetDir);
				if (d->exec() == QDialog::Accepted)
				{
					mvCpTargetDir = d->getTargetDir();
				}
				delete d;
			}
			break;
		case ACT_FILE_MOVE:
			{
				QDir dir;
				if (mvCpTargetDir.isEmpty() || !dir.exists(mvCpTargetDir))
				{
					QMessageBox::critical(this, tr("Error"), tr("Target directory not set."));
				} else
				if (currentDirPath == mvCpTargetDir)
				{
					QMessageBox::critical(this, tr("Error"), tr("Target directory cannot be the current one."));
				}
				else
				{
					QFile file(currentDirPath+"/"+currentFilePath);
					file.copy(mvCpTargetDir+"/"+currentFilePath);
					if (file.remove())
					{
						int index = indexedFiles.indexOf(currentFilePath);
						indexedFiles.removeAt(index);
						if (indexedFiles.length() == 0) //deleted the last file
						{
							sendAction(ACT_UNLOAD);
						}
						else
						{
							if (index >= indexedFiles.length()) index -= 1;
							currentFilePath = indexedFiles.at(index);
							if (thumbnailDialog)
							{
								thumbnailDialog->updateFileList(indexedDirPath, indexedFiles);
							}
							dirCountDisplay->setText(QString("%1").arg(indexedFiles.length()));
							setIndexDisplayNoSignals(index+1, indexedFiles.length());
							loadCurrentFile();
						}
					}
				}
			}
			break;
		case ACT_FILE_COPY:
			{
				QDir dir;
				if (mvCpTargetDir.isEmpty() || !dir.exists(mvCpTargetDir))
				{
					QMessageBox::critical(this, tr("Error"), tr("Target directory not set."));
				} else
				if (currentDirPath == mvCpTargetDir)
				{
					QMessageBox::critical(this, tr("Error"), tr("Target directory cannot be the current one."));
				}
				else
				{
					QFile file(currentDirPath+"/"+currentFilePath);
					file.copy(mvCpTargetDir+"/"+currentFilePath);
				}
			}
			break;
		case ACT_FILE_DELETE:
			{
				bool deleted = false;
				int index = indexedFiles.indexOf(currentFilePath);
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
				QProcess moveToTrashProc;
				moveToTrashProc.setStandardErrorFile(QProcess::nullDevice());
				moveToTrashProc.setStandardOutputFile(QProcess::nullDevice());
				if (moveToTrashWithGIO)
				{
					QStringList arguments;
					arguments.append("trash");
					arguments.append(currentDirPath+"/"+currentFilePath);
					moveToTrashProc.start("gio", arguments);
					deleted = (!(!moveToTrashProc.waitForFinished(1000) || (moveToTrashProc.error() == QProcess::FailedToStart)));
				} else
				if (moveToTrashWithKIO)
				{
					QStringList arguments;
					arguments.append("mv");
					arguments.append(currentDirPath+"/"+currentFilePath);
					arguments.append("trash:/");
					moveToTrashProc.start("kioclient5", arguments);
					deleted = (!(!moveToTrashProc.waitForFinished(1000) || (moveToTrashProc.error() == QProcess::FailedToStart)));
				}
				if (deleted)
				{
					QFileInfo file(currentDirPath+"/"+currentFilePath);
					if (file.exists())
					{
						deleted = false;
					}
				}
				if (!deleted)
				{
					if (moveToTrashWithGIO || moveToTrashWithKIO)
					{
						QMessageBox::critical(this, tr("Error"), QString(tr("Failed to move %1 to trash.")).arg(currentFilePath));
					}
					else
					{
						QMessageBox::critical(this, tr("Error"), QString(tr("Failed to move %1 to trash.\nCurrently only 'gio' and 'kioclient5' based operation supported.")).arg(currentFilePath));
					}
				}

#else
				QFile fileToBeRemoved(currentDirPath+"/"+currentFilePath);
				deleted = fileToBeRemoved.moveToTrash();
#endif
				if (deleted)
				{
					indexedFiles.removeAt(index);
					if (indexedFiles.length() == 0) //deleted the last file
					{
						sendAction(ACT_UNLOAD);
					}
					else
					{
						if (index >= indexedFiles.length()) index -= 1;
						currentFilePath = indexedFiles.at(index);
						if (thumbnailDialog)
						{
							thumbnailDialog->updateFileList(indexedDirPath, indexedFiles);
						}
						dirCountDisplay->setText(QString("%1").arg(indexedFiles.length()));
						setIndexDisplayNoSignals(index+1, indexedFiles.length());
						loadCurrentFile();
					}
				}
			}
			break;
		case ACT_SAVE:
			{
				SaveFileDialog::ReturnCluster cluster = fileDialogSave(currentDirPath, currentImageName);
				if (!cluster.filePath.isEmpty())
				{
					QApplication::setOverrideCursor(Qt::WaitCursor);
					bool success = imageIO->saveFile(cluster.filePath, cluster.selectedFormat, display->getImage(), cluster.parameters);
					QApplication::restoreOverrideCursor();
					if (!success)
					{
						QMessageBox::critical(this, tr("Error"), QString(tr("Save to %1 failed.")).arg(cluster.filePath));
					}
				}
			}
			break;
		case ACT_SAVE_AS:
		case ACT_SAVE_SELECTION_AS:
			{
				SaveFileDialog::ReturnCluster cluster = fileDialogSave(QString(), currentImageName, true);
				if (!cluster.filePath.isEmpty())
				{
					QFileInfo file(cluster.filePath);
					Globals::prefs->setLastSaveAsDir(file.absolutePath());
					QApplication::setOverrideCursor(Qt::WaitCursor);
					QImage image;
					if (a == ACT_SAVE_SELECTION_AS)
					{
						image = display->getFromSelection();
					}
					else
					{
						image = display->getImage();
					}
					bool success = imageIO->saveFile(cluster.filePath, cluster.selectedFormat, image, cluster.parameters);
					QApplication::restoreOverrideCursor();
					if (!success)
					{
						QMessageBox::critical(this, tr("Error"), QString(tr("Save to %1 failed.")).arg(cluster.filePath));
					}
				}
			}
			break;
		case ACT_PRINT:
			{
				QPrinter printer;
				printer.setColorMode(QPrinter::Color);
				QSize imageSize = display->getImageSize();
				if (imageSize.width() > imageSize.height())
				{
					printer.setPageOrientation(QPageLayout::Landscape);
				}
				QPrintDialog * d = new QPrintDialog(&printer);
				if (d->exec() == QDialog::Accepted)
				{
					QPainter painter;
					if (painter.begin(&printer))
					{
						QPixmap pixmap = QPixmap::fromImage(display->getImage());
						QRect rect = painter.viewport();
						QSize size = pixmap.size();
						size.scale(rect.size(), Qt::KeepAspectRatio);
						painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
						painter.setWindow(pixmap.rect());
						painter.drawPixmap(0, 0, pixmap);
						painter.end();
					}
					else
					{
						QMessageBox::critical(this, tr("Error"), QString(tr("Printer initialization failed.")));
					}
				}
				delete d;
			}
			break;
		case ACT_EXIT:
			if (isFullscreen)
			{
				slideshowDirection = 0;
				slideshowTimer.stop();
				setFullscreen(false);
			} else
			if (thumbnailDialog)
			{
				deleteThumbnailDialog();
			}
			else
			{
				if (Globals::prefs->getClearClipboardOnExit())
				{
					clearClipboard();
				}
				Globals::prefs->setWindowPosition(this->pos());
				qApp->quit();
			}
			break;
		case ACT_UNDO:
			undoFromUndoStack();
			break;
		case ACT_REDO:
			redoFromUndoStack();
			break;
		case ACT_CUSTOM_SELECTION:
			{
				CustomSelectionDialog * d = new CustomSelectionDialog(display->getImageSize(), display->getSelection());
				if (d->exec() == QDialog::Accepted)
				{
					d->savePreferences();
					QRect newSelection = d->getSelection();
					display->setSelection(newSelection);
				}
				delete d;
			}
			break;
		case ACT_CUT_SELECTION:
			if (!display->getSelection().isNull())
			{
				addToClipboard(display->getFromSelection());
				QImage insert = QImage(1, 1, QImage::Format_RGB32);
				insert.fill(0);
				saveToUndoStack();
				display->insertIntoSelection(insert);
			}
			break;
		case ACT_CROP_SELECTION:
			{
				QImage i = display->getFromSelection();
				saveToUndoStack();
				display->newImage(i);
				currentImageSize = i.size();
				setImageAndWindowSize();
			}
			break;
		case ACT_COPY:
			addToClipboard(display->getFromSelection());
			break;
		case ACT_PASTE:
			if (clipboardHasImage())
			{
				QApplication::setOverrideCursor(Qt::WaitCursor);
				QImage i = getFromClipboard();
				QApplication::restoreOverrideCursor();
				if (i.isNull())
				{
					QMessageBox::critical(this, tr("Error"), QString(tr("QClipboard error.")));
				}
				else
				{
					if (display->getSelection().isNull())
					{
						setInternalState(IMAGE_FROM_CLIPBOARD);
						clipboardCounter += 1;
						currentImageName = QString(tr("Clipboard %1")).arg(clipboardCounter);
						clearPastedUndoStack();
						display->newImage(i);
						currentImageSize = i.size();
						if (Globals::prefs->getDisplayMode() == 0)
						{
							display->setZoom(1.0);
						}
						setImageAndWindowSize();
					}
					else
					{
						saveToUndoStack();
						display->insertIntoSelection(getFromClipboard());
					}
				}
			}
			break;
		case ACT_PASTE_TO_SIDE:
			if (clipboardHasImage())
			{
				QApplication::setOverrideCursor(Qt::WaitCursor);
				QImage i = getFromClipboard();
				QApplication::restoreOverrideCursor();
				if (i.isNull())
				{
					QMessageBox::critical(this, tr("Error"), QString(tr("QClipboard error.")));
				}
				else
				{
					PasteToSideDialog * d = new PasteToSideDialog();
					if (d->exec() == QDialog::Accepted)
					{
						d->savePreferences();
						QImage displayedImg = display->getImage();
						QImage jointImage;
						int side = d->getSide();
						if (side % 2)	// TB
						{
							if (currentImageSize.width() != i.width())
							{
								double newHeight = i.height();
								newHeight *= currentImageSize.width();
								newHeight /= i.width();
								i = i.scaled(currentImageSize.width(), std::round(newHeight), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
							}
							jointImage = QImage(currentImageSize.width(), currentImageSize.height()+i.height(), displayedImg.format());
						}
						else	// LR
						{
							if (currentImageSize.height() != i.height())
							{
								double newWidth = i.width();
								newWidth *= currentImageSize.height();
								newWidth /= i.height();
								i = i.scaled(std::round(newWidth), currentImageSize.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
							}
							jointImage = QImage(currentImageSize.width() + i.width(), currentImageSize.height(), displayedImg.format());
						}
						QPainter painter(&jointImage);
						painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
						switch (side)
						{
							case 1:	// top
								painter.drawImage(QPoint(0, i.height()), displayedImg);
								painter.drawImage(QPoint(0, 0), i);
								break;
							case 2:	// right
								painter.drawImage(QPoint(0, 0), displayedImg);
								painter.drawImage(QPoint(displayedImg.width(), 0), i);
								break;
							case 3:	// bottom
								painter.drawImage(QPoint(0, 0), displayedImg);
								painter.drawImage(QPoint(0, displayedImg.height()), i);
								break;
							case 4:	// left
								painter.drawImage(QPoint(i.width(), 0), displayedImg);
								painter.drawImage(QPoint(0, 0), i);
								break;
							default: break;
						}
						painter.end();
						saveToUndoStack();
						display->newImage(jointImage);
						currentImageSize = jointImage.size();
						setImageAndWindowSize();
					}
					delete d;
				}
			}
			break;
		case ACT_UNLOAD:
			if (isFullscreen)
			{
				setFullscreen(false);
			}
			setInternalState(UNLOADED);
			display->newImage(QImage());
			currentImageSize = QSize();
			currentImageName = QString();
			currentFilePath = QString();
			currentDirPath = QString();
			indexedDirPath = QString();
			indexedFiles = QStringList();
			zoomDisplay->setText("");
			indexDisplay->setMinimum(1);
			indexDisplay->setMaximum(1);
			dirCountDisplay->setText("");
			clearUndoStack();
			break;
		case ACT_CLEAR_CLIPBOARD:
			clearClipboard(true);
			break;
		case ACT_INFO:
			if (internalState == IMAGE_FROM_FILE)
			{
				InfoDialog * d = new InfoDialog(currentDirPath + "/" + currentFilePath);
				d->exec();
				delete d;
			}
			break;
		case ACT_ROTATE_L:
			doSimpleFilter(ROT_L);
			break;
		case ACT_ROTATE_R:
			doSimpleFilter(ROT_R);
			break;
		case ACT_ROTATE_C:
			{
				RotateDialog * d = new RotateDialog(display->getImage());
				if (d->exec() == QDialog::Accepted)
				{
					QApplication::setOverrideCursor(Qt::WaitCursor);
					QImage i = d->rotateImage(display->getImage());
					QApplication::restoreOverrideCursor();
					saveToUndoStack();
					display->newImage(i);
					currentImageSize = i.size();
					setImageAndWindowSize();
					d->savePreferences();
				}
				delete d;
			}
			break;
		case ACT_FLIP_V:
			doSimpleFilter(MIRROR_V);
			break;
		case ACT_FLIP_H:
			doSimpleFilter(MIRROR_H);
			break;
		case ACT_RESIZE:
			{
				ResizeDialog * d = new ResizeDialog(currentImageSize.width(), currentImageSize.height());
				if (d->exec() == QDialog::Accepted)
				{
					QApplication::setOverrideCursor(Qt::WaitCursor);
					QImage i = Resizer::Resize(display->getImage(), d->getNewResolution(), d->getAlgorithm());
					QApplication::restoreOverrideCursor();
					saveToUndoStack();
					display->newImage(i);
					currentImageSize = d->getNewResolution();//i.size();
					setImageAndWindowSize();
					d->savePreferences();
				}
				delete d;
			}
			break;
		case ACT_ADD_BORDER:
			{
				AddBorderDialog * d = new AddBorderDialog();
				if (d->exec() == QDialog::Accepted)
				{
					QApplication::setOverrideCursor(Qt::WaitCursor);
					QImage i = d->addBorder(display->getImage());
					QApplication::restoreOverrideCursor();
					saveToUndoStack();
					display->newImage(i);
					currentImageSize = i.size();
					setImageAndWindowSize();
					d->savePreferences();
				}
				delete d;
			}
			break;
		case ACT_PAD_TO_SIZE:
			{
				PadToSizeDialog * d = new PadToSizeDialog();
				if (d->exec() == QDialog::Accepted)
				{
					QApplication::setOverrideCursor(Qt::WaitCursor);
					QImage i = d->padToSize(display->getImage());
					QApplication::restoreOverrideCursor();
					saveToUndoStack();
					display->newImage(i);
					currentImageSize = i.size();
					setImageAndWindowSize();
					d->savePreferences();
				}
				delete d;
			}
			break;
		case ACT_COLOR_DEPTH_INC:
			{
				QApplication::setOverrideCursor(Qt::WaitCursor);
				saveToUndoStack();
				QImage i = display->getImage();
				if (i.hasAlphaChannel())
				{
					i = i.convertToFormat(QImage::Format_ARGB32).copy();
				}
				else
				{
					i = i.convertToFormat(QImage::Format_RGB32).copy();
				}
				display->updateImage(i);
				QApplication::restoreOverrideCursor();
			}
			break;
		case ACT_COLOR_DEPTH_DEC:
			{
				EffectsDialog * d = new EffectsDialog(display->getImage(), QString(tr("Color Quantization")));
				if (d->exec() == QDialog::Accepted)
				{
					QApplication::setOverrideCursor(Qt::WaitCursor);
					saveToUndoStack();
					QImage i = d->applyEffects();
					display->updateImage(i);
					QApplication::restoreOverrideCursor();
				}
				delete d;
			}
			break;
		case ACT_GRAYSCALE:
			doSimpleFilter(GRAYS);
			break;
		case ACT_NEGATIVE:
			doSimpleFilter(NEGATIVE);
			break;
		case ACT_COLOR_ADJUST:
			{
				ColorAdjDialog * d = new ColorAdjDialog(display->getImage());
				if (d->exec() == QDialog::Accepted)
				{
					QApplication::setOverrideCursor(Qt::WaitCursor);
					QImage i = d->adjustColor(display->getImage());
					QApplication::restoreOverrideCursor();
					saveToUndoStack();
					display->newImage(i);
					currentImageSize = i.size();
					setImageAndWindowSize();
					d->savePreferences();
				}
				delete d;
			}
			break;
		case ACT_AUTO_COLOR:
			doSimpleFilter(COLORADJ);
			break;
		case ACT_SHARPEN:
			doSimpleFilter(SHARPEN);
			break;
		case ACT_EFFECTS:
			{
				EffectsDialog * d = new EffectsDialog(display->getFromSelection());
				if (d->exec() == QDialog::Accepted)
				{
					QApplication::setOverrideCursor(Qt::WaitCursor);
					QImage i = d->applyEffects();
					QApplication::restoreOverrideCursor();
					saveToUndoStack();
					if (display->getSelection().isNull())
					{
						display->updateImage(i);
						if (currentImageSize != i.size())
						{
							currentImageSize = i.size();
							setImageAndWindowSize();
						}
					}
					else
					{
						display->insertIntoSelection(i);
					}
				}
				delete d;
			}
			break;
		case ACT_EXTERNAL_TOOL_CONFIG:
			{
				ExtToolConfigDialog * d = new ExtToolConfigDialog();
				if (d->exec() == QDialog::Accepted)
				{
					d->savePreferences();
				}
				delete d;
			}
			break;
		case ACT_EXTERNAL_TOOL_1:
		case ACT_EXTERNAL_TOOL_2:
		case ACT_EXTERNAL_TOOL_3:
		case ACT_EXTERNAL_TOOL_4:
		case ACT_EXTERNAL_TOOL_5:
		case ACT_EXTERNAL_TOOL_6:
		case ACT_EXTERNAL_TOOL_7:
		case ACT_EXTERNAL_TOOL_8:
		case ACT_EXTERNAL_TOOL_9:
			{
				int index = (a - ACT_EXTERNAL_TOOL_1) + 1;
				QString shell = Globals::prefs->fetchSpecificParameter(QString("ExtToolConfig%1").arg(index), "shell", "").toString();
				int type = Globals::prefs->fetchSpecificParameter(QString("ExtToolConfig%1").arg(index), "type", 0).toInt();
				QString path = Globals::prefs->fetchSpecificParameter(QString("ExtToolConfig%1").arg(index), "path", "").toString();
				QString inputFilePath, outputFilePath;
				QProcess process;
				do {
					if (shell.isEmpty() || path.isEmpty())
					{
						QMessageBox::critical(this, tr("Error"), QString(tr("External tool %1 is not configured.")).arg(index));
						break;
					}
					
					QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddHHmmsszzz");
					inputFilePath =  "/tmp/civ_exttool_in_" + timestamp;
					outputFilePath = "/tmp/civ_exttool_out_" + timestamp;
					
					QApplication::setOverrideCursor(Qt::WaitCursor);
					QImage img = display->getFromSelection();
					bool success = false;
					switch(type)
					{
						case 0:
							inputFilePath += ".bmp";
							outputFilePath += ".bmp";
							success = imageIO->saveFile(inputFilePath, "bmp", img, QList<IObase::ParameterCluster>());
							break;
						case 1:
							inputFilePath += ".jpg";
							outputFilePath += ".jpg";
							success = imageIO->saveFile(inputFilePath, "jpg", img, QList<IObase::ParameterCluster>());
							break;
						case 2:
							inputFilePath += ".png";
							outputFilePath += ".png";
							success = imageIO->saveFile(inputFilePath, "png", img, QList<IObase::ParameterCluster>());
							break;
					}
					if (!success)
					{
						QMessageBox::critical(this, tr("Error"), QString(tr("Failed to save the script input file.")));
						break;
					}
					
					process.setProgram(shell);
					process.setArguments({path, inputFilePath, outputFilePath});
					process.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
					process.setProcessChannelMode(QProcess::MergedChannels);
					process.start();
					if (!process.waitForStarted(1000))
					{
						QMessageBox::critical(this, tr("Error"), QString(tr("External tool %1 failed to start.")).arg(index));
						break;
					}
					if (!process.waitForFinished(200))
					{
						QMessageBox abortDialog(QMessageBox::Warning, QString(tr("External tool %1")).arg(index), 
											QString(tr("The execution of the script is in progress...")),
											QMessageBox::Abort);
						abortDialog.setEscapeButton(QMessageBox::Abort);
						abortDialog.button(QMessageBox::Abort)->setCheckable(true);
						abortDialog.open();
						QCoreApplication::processEvents();
						bool aborted = false;
						while(!process.waitForFinished(20))
						{
							QCoreApplication::processEvents();
							
							if (abortDialog.button(QMessageBox::Abort)->isChecked())
							{
								aborted = true;
								break;
							}
						}
						if (aborted) break;
					}
					img = imageIO->loadFile(outputFilePath);
					if (img.isNull())
					{
						QMessageBox::critical(this, tr("Error"), QString(tr("Failed to load the script output file.")));
						break;
					}
					
					saveToUndoStack();
					if (display->getSelection().isNull())
					{
						display->updateImage(img);
						if (currentImageSize != img.size())
						{
							currentImageSize = img.size();
							setImageAndWindowSize();
						}
					}
					else
					{
						display->insertIntoSelection(img);
					}
				} while(0);
				if (!inputFilePath.isEmpty())
				{
					remove(inputFilePath.toUtf8().constData());
				}
				if (!outputFilePath.isEmpty())
				{
					remove(outputFilePath.toUtf8().constData());
				}
				process.close();
				QApplication::restoreOverrideCursor();
			}
			break;
		case ACT_SETTINGS:
			{
				int prevMode = Globals::prefs->getDisplayMode();
				PreferencesDialog * d = new PreferencesDialog();
				if (d->exec() == QDialog::Accepted)
				{
					d->savePreferences();
					indexDisplay->setWrapping(Globals::prefs->getLoopDir());
					display->setBackgroundShade(Globals::prefs->getDisplayBackground());
					ui.toolBar->setSizePolicy((Globals::prefs->getEnableToolbarShrinking() ? (QSizePolicy::Expanding) : (QSizePolicy::MinimumExpanding)), QSizePolicy::Minimum);
					if (prevMode != Globals::prefs->getDisplayMode())
					{
						if (internalState != UNLOADED)
						{
							display->setZoom(1.0);
							setImageAndWindowSize();
						}
					}
					updateDisplayModeMenu();
				}
				delete d;
			}
			break;
		case ACT_MINIMIZE:
			setWindowState((windowState() & ~Qt::WindowActive) | Qt::WindowMinimized);
			break;
		case ACT_TOGGLE_STATUSBAR:
			ui.statusBar->setVisible(! ui.statusBar->isVisible());
			Globals::prefs->setStatusbarVisible(ui.statusBar->isVisible());
			break;
		case ACT_TOGGLE_TOOLBAR:
			ui.toolBar->setVisible(! ui.toolBar->isVisible());
			Globals::prefs->setToolbarVisible(ui.toolBar->isVisible());
			break;
		case ACT_TOGGLE_MENUBAR:
			ui.menuBar->setVisible(! ui.menuBar->isVisible());
			Globals::prefs->setMenubarVisible(ui.menuBar->isVisible());
			break;
		case ACT_TOGGLE_FULLSCREEN:
			slideshowDirection = 0;
			slideshowTimer.stop();
			setFullscreen(!isFullscreen);
			break;
		case ACT_DISPLAY_MODE_0:
		case ACT_DISPLAY_MODE_1:
		case ACT_DISPLAY_MODE_2:
		case ACT_DISPLAY_MODE_3:
		case ACT_DISPLAY_MODE_4:
		case ACT_DISPLAY_MODE_5:
			{
				int mode = (a - ACT_DISPLAY_MODE_0);
				if ((mode > 3) && (mode == Globals::prefs->getDisplayMode()))
				{
					mode = 0;
				}
				Globals::prefs->setDisplayMode(mode);
				if (internalState != UNLOADED)
				{
					display->setZoom(1.0);
					setImageAndWindowSize();
				}
				updateDisplayModeMenu();
			}
			break;
		case ACT_NEXT_FILE:
			{
				if (indexedFiles.length() > 0)
				{
					int index = indexedFiles.indexOf(currentFilePath);
					index++;
					if (index >= indexedFiles.size()) 
					{
						if (Globals::prefs->getLoopDir())
						{
							index = 0;
						}
						else
						{
							index = indexedFiles.size() - 1;
						}
					}
					currentFilePath = indexedFiles.at(index);
					loadCurrentFile();
				}
			}
			break;
		case ACT_PREV_FILE:
			{
				if (indexedFiles.length() > 0)
				{
					int index = indexedFiles.indexOf(currentFilePath);
					index--;
					if (index < 0)
					{
						if (Globals::prefs->getLoopDir())
						{
							index = indexedFiles.size() - 1;
						}
						else
						{
							index = 0;
						}
					}
					currentFilePath = indexedFiles.at(index);
					loadCurrentFile();
				}
			}
			break;
		case ACT_FIRST_FILE:
			{
				if (indexedFiles.length() > 0)
				{
					currentFilePath = indexedFiles.at(0);
					loadCurrentFile();
				}
			}
			break;
		case ACT_LAST_FILE:
			{
				if (indexedFiles.length() > 0)
				{
					currentFilePath = indexedFiles.at(indexedFiles.size() - 1);
					loadCurrentFile();
				}
			}
			break;
		case ACT_ZOOM_IN:
			{
				double z = display->getZoom();
				double zoomDelta = Globals::prefs->getZoomDelta();
				zoomDelta = (zoomDelta/100.0) + 1.0;
				z *= zoomDelta;
				if (z > 1000.0) z = 1000.0;
				display->setZoom(z);
			}
			break;
		case ACT_ZOOM_OUT:
			{
				double z = display->getZoom();
				double zoomDelta = Globals::prefs->getZoomDelta();
				zoomDelta = (zoomDelta/100.0) + 1.0;
				z /= zoomDelta;
				if (z < 0.001) z = 0.001;
				display->setZoom(z);
			}
			break;
		case ACT_ZOOM_1:
			display->setZoom(1.0);
			break;
		case ACT_ZOOM_5:
			display->setZoom(0.05);
			break;
		case ACT_ZOOM_10:
			display->setZoom(0.1);
			break;
		case ACT_ZOOM_15:
			display->setZoom(0.15);
			break;
		case ACT_ZOOM_20:
			display->setZoom(0.2);
			break;
		case ACT_ZOOM_25:
			display->setZoom(0.25);
			break;
		case ACT_ZOOM_33:
			display->setZoom(0.33);
			break;
		case ACT_ZOOM_50:
			display->setZoom(0.5);
			break;
		case ACT_ZOOM_66:
			display->setZoom(0.66);
			break;
		case ACT_ZOOM_100:
			display->setZoom(1.0);
			break;
		case ACT_ZOOM_125:
			display->setZoom(1.25);
			break;
		case ACT_ZOOM_150:
			display->setZoom(1.5);
			break;
		case ACT_ZOOM_175:
			display->setZoom(1.75);
			break;
		case ACT_ZOOM_200:
			display->setZoom(2.0);
			break;
		case ACT_ZOOM_300:
			display->setZoom(3.0);
			break;
		case ACT_ZOOM_400:
			display->setZoom(4.0);
			break;
		case ACT_LICENSE:
			{
				LicenseDialog * d = new LicenseDialog();
				d->exec();
				delete d;
			}
			break;
		case ACT_SHORTCUTS:
			{
				ShortcutsDialog * d = new ShortcutsDialog(shortCutInfo);
				d->exec();
				delete d;
			}
			break;
		case ACT_ABOUT:
			{
				AboutDialog * d = new AboutDialog();
				d->exec();
				delete d;
			}
			break;
		default:
			qDebug() << "Invalid internal Action: " << a;
			break;
	}
	actionLock--;
}

void MainWindow::searchAction(QAction * action)
{
	for (const ActionLookUp_t & element : ActionLookUpTable)
	{
		if (element.actionRef == action)
		{
			sendAction(element.event);
		}
	}
}

QAction * MainWindow::searchQAction(Action a)
{
	for (const ActionLookUp_t & element : ActionLookUpTable)
	{
		if (element.event == a)
		{
			return element.actionRef;
		}
	}
	return NULL;
}

void MainWindow::removeAction(Action a)
{
	std::vector<ActionLookUp_t> oldTable = ActionLookUpTable;
	ActionLookUpTable.clear();
	ActionLookUpTable.reserve(oldTable.size());
	for (const ActionLookUp_t & element : oldTable)
	{
		if (element.event != a)
		{
			ActionLookUpTable.push_back(element);
		}
	}
}

void MainWindow::addToClipboard(QImage image)
{
	if (image.isNull()) return;
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setImage(image);
}

void MainWindow::clearClipboard(bool forced)
{
	QClipboard *clipboard = QApplication::clipboard();
	// only clear my image
	if (clipboard->ownsClipboard() || forced)
	{
		clipboard->clear();
	}
}

bool MainWindow::clipboardHasImage()
{
	QClipboard *clipboard = QApplication::clipboard();
	return clipboard->mimeData()->hasImage();
}

QImage MainWindow::getFromClipboard()
{
	QClipboard *clipboard = QApplication::clipboard();
	QImage img = clipboard->image();
	if (img.isNull())	// ugly hack to bypass Qt bug
	{
		QProgressDialog progress(tr("Trying to hack the clipboard..."), "Cancel", 0, 1024, this);
		ui.statusBar->showMessage(tr("Trying to hack the clipboard..."));
		progress.setWindowModality(Qt::WindowModal);
		progress.show();
		for (int i=0; i<1024; i++)
		{
			progress.setValue(i);
			QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
			img = clipboard->image();
			if (!img.isNull()) break;
			if (!clipboard->mimeData()->hasImage()) break;
			if (progress.wasCanceled()) break;
		}
		progress.done(0);
		ui.statusBar->clearMessage();
	}
	return img;
}

void MainWindow::displayNeedNextImage()
{
	sendAction(ACT_NEXT_FILE);
}

void MainWindow::displayNeedPrevImage()
{
	sendAction(ACT_PREV_FILE);
}

void MainWindow::displayNeedFirstImage()
{
	sendAction(ACT_FIRST_FILE);
}

void MainWindow::displayNeedLastImage()
{
	sendAction(ACT_LAST_FILE);
}


void MainWindow::displayZoomChanged()
{
	zoomDisplay->setText(QString("%1 %").arg(display->getZoom()*100.0, 0, 'f', 1));
	QSize imageSize = display->getImageSize();
	if (imageSize.isNull())
	{
		statusBarResolution->setText(QString(tr("No file loaded")));
		clearFileModificationTime();
		statusBarIndex->setText("-");
		statusBarZoom->setText("");
	}
	else
	{
		statusBarResolution->setText(QString("%1 x %2 x %3BPP").arg(imageSize.width()).arg(imageSize.height()).arg(display->getImageBpp()));
		statusBarZoom->setText(QString("%1%").arg(display->getZoom()*100.0, 0, 'f', 0));
	}
	if (Globals::prefs->getFitWindowWhenZoomed())
	{
		if (Globals::prefs->getDisplayMode() < 3)
		{
			setWindowSize();
		}
	}
}

void MainWindow::displaySelectionChanged()
{
	QRect selection = display->getSelection();
	if (selection.isNull())
	{
		statusBarSelection->setText(QString(tr("No selection")));
	}
	else
	{
		statusBarSelection->setText(QString(tr("Selection: %1x%2 at (%3,%4)")).arg(selection.width()).arg(selection.height()).arg(selection.x()).arg(selection.y()));
	}
	
	
}

void MainWindow::displayPixelInfo()
{
	QColor color = display->getPixelInfoColor();
	bool hasAlpha = display->getPixelInfoHasAlpha();
	QPoint pos = display->getPixelInfoPos();
	if (color.isValid())
	{
		int r,g,b,a;
		color.getRgb(&r, &g, &b, &a);
		QString pixelColor = QString();
		if (hasAlpha)
		{
			pixelColor = QString("(%1,%2,%3,%4), %5").arg(r).arg(g).arg(b).arg(a).arg(color.name(QColor::HexArgb));
		}
		else
		{
			pixelColor = QString("(%1,%2,%3), %4").arg(r).arg(g).arg(b).arg(color.name(QColor::HexRgb));
		}
		statusBarSelection->setText(QString(tr("Pixel: %1 at (%2,%3)")).arg(pixelColor).arg(pos.x()).arg(pos.y()));
	}
}

void MainWindow::indexDisplayChanged(int i)
{
	if (i <= 0) return;
	if (i > indexedFiles.size()) return;
	currentFilePath = indexedFiles.at(i-1);
	loadCurrentFile();
	indexDisplay->clearFocus();
	// lose focus:
	indexDisplay->clearFocus();
	display->setFocus();
}

void MainWindow::setIndexDisplayNoSignals(int value, int maximum)
{
	bool oldState = indexDisplay->blockSignals(true);
	indexDisplay->setMinimum(1);
	if (maximum > 0)
	{
		indexDisplay->setMaximum(maximum);
	}
	indexDisplay->setValue(value);
	indexDisplay->blockSignals(oldState);
}

void MainWindow::slideshowTimeout()
{
	if (slideshowDirection < 0)
	{
		sendAction(ACT_PREV_FILE);
	} else
	if (slideshowDirection > 0)
	{
		sendAction(ACT_NEXT_FILE);
	}
}

void MainWindow::deleteThumbnailDialog()
{
	if (thumbnailDialog)
	{
		disconnect(thumbnailDialog, SIGNAL(finished(int)), NULL, NULL);
		if (thumbnailDialog->isVisible())
		{
			thumbnailDialog->close();
		}
		delete thumbnailDialog;
		thumbnailDialog = NULL;
	}
}

void MainWindow::thumbnailDialogClosed(int i)
{
	Q_UNUSED(i);
	deleteThumbnailDialog();
}

void MainWindow::thumbnailItemSelected(int index)
{
	this->activateWindow();
	indexDisplay->setValue(index+1);
}

void MainWindow::setupToolBar()
{
	QAction * a;
	#define ADD_ACTION(ACT, ICON) if ((a = searchQAction(ACT))) \
					{ \
						ui.toolBar->addAction(a); \
						a->setIcon(QIcon(ICON)); \
					}
				
	ADD_ACTION(ACT_OPEN, ":/icons/icons/open.png");
	ADD_ACTION(ACT_SAVE, ":/icons/icons/save.png");
	ui.toolBar->addSeparator();
	ADD_ACTION(ACT_CUT_SELECTION, ":/icons/icons/cut.png");
	ADD_ACTION(ACT_COPY, ":/icons/icons/copy.png");
	ADD_ACTION(ACT_PASTE, ":/icons/icons/paste.png");
	ADD_ACTION(ACT_UNDO, ":/icons/icons/undo.png");
	ADD_ACTION(ACT_REDO, ":/icons/icons/redo.png");
	ui.toolBar->addSeparator();
	ADD_ACTION(ACT_INFO, ":/icons/icons/info.png");
	ADD_ACTION(ACT_ZOOM_IN, ":/icons/icons/zoom-in.png");
	ADD_ACTION(ACT_ZOOM_OUT, ":/icons/icons/zoom-out.png");
	ADD_ACTION(ACT_ZOOM_1, ":/icons/icons/zoom-1.png");
	
	zoomDisplay = new QLineEdit();
	zoomDisplay->setAlignment(Qt::AlignRight);
	zoomDisplay->setReadOnly(true);
	zoomDisplay->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	zoomDisplay->setFixedWidth(zoomDisplay->fontMetrics().boundingRect(QString("999999.9 %")).width() * 1.1);
	// make unselectable
	QPalette enabledPalette = zoomDisplay->palette();
	QColor baseColor = enabledPalette.color(QPalette::Base);
	QColor textColor = enabledPalette.color(QPalette::Text);
	zoomDisplay->setDisabled(true);
	enabledPalette.setColor(QPalette::Base, baseColor);
	enabledPalette.setColor(QPalette::Text, textColor);
	zoomDisplay->setPalette(enabledPalette);
	zoomDisplay->installEventFilter(this);
	ui.toolBar->addWidget(zoomDisplay);
	
	ui.toolBar->addSeparator();
	ADD_ACTION(ACT_PREV_FILE, ":/icons/icons/backward.png");
	ADD_ACTION(ACT_NEXT_FILE, ":/icons/icons/forward.png");
	
	indexDisplay = new QSpinBox();
	indexDisplay->setMinimum(1);
	indexDisplay->setMaximum(1);
	indexDisplay->setSingleStep(1);
	indexDisplay->setWrapping(Globals::prefs->getLoopDir());
	indexDisplay->setKeyboardTracking(false);
	QLabel * slash = new QLabel(QString("/"));
	dirCountDisplay = new QLineEdit();
	dirCountDisplay->setAlignment(Qt::AlignHCenter);
	dirCountDisplay->setReadOnly(true);
	dirCountDisplay->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	dirCountDisplay->setFixedWidth(zoomDisplay->fontMetrics().boundingRect(QString("9999999")).width() * 1.5);
	dirCountDisplay->setDisabled(true);
	dirCountDisplay->setPalette(enabledPalette);
	ui.toolBar->addWidget(indexDisplay);
	ui.toolBar->addWidget(slash);
	ui.toolBar->addWidget(dirCountDisplay);

	zoomDisplay->adjustSize();
	indexDisplay->adjustSize();
	dirCountDisplay->adjustSize();
	indexDisplay->installEventFilter(this);
	connect(indexDisplay, SIGNAL(valueChanged(int)), this, SLOT(indexDisplayChanged(int)));
	#undef ADD_ACTION
}

void MainWindow::setupStatusBar()
{
	statusBarResolution = new QLabel("");
	statusBarIndex = new QLabel("");
	statusBarZoom = new QLabel("");
	statusBarLastModified = new QLabel("");
	statusBarSelection = new QLabel("");
	QFrame * separator;
	#define ADD_STATUSBAR_SEPARATOR()		separator = new QFrame(); separator->setFrameStyle(QFrame::VLine | QFrame::Sunken); ui.statusBar->addWidget(separator);	// due to fusion style
	ui.statusBar->addWidget(statusBarResolution);
	ADD_STATUSBAR_SEPARATOR();
	ui.statusBar->addWidget(statusBarIndex);
	ADD_STATUSBAR_SEPARATOR();
	ui.statusBar->addWidget(statusBarZoom);
	ADD_STATUSBAR_SEPARATOR();
	ui.statusBar->addWidget(statusBarLastModified);
	ADD_STATUSBAR_SEPARATOR();
	ui.statusBar->addWidget(statusBarSelection);
	#undef ADD_STATUSBAR_SEPARATOR
	statusBarResolution->setMinimumWidth(1);	// make QLabel elastic
	statusBarIndex->setMinimumWidth(1);
	statusBarZoom->setMinimumWidth(1);
	statusBarLastModified->setMinimumWidth(1);
	statusBarSelection->setMinimumWidth(1);
	
	statusBarIndex->setText("-");
	statusBarResolution->setText(QString(tr("No file loaded")));
	statusBarSelection->setText(QString(tr("No selection")));
}

QStringList MainWindow::fastIndexer(QString dirPath, QStringList extensions, int ordering)
{
	std::string dirPathS = dirPath.toUtf8().constData();
	DIR * dirp = opendir(dirPathS.c_str());
	if (dirp == NULL) return QStringList();
	
	std::string fullPath = dirPathS + "/";
	size_t fullPathLen = fullPath.length();
	fullPath.reserve(fullPathLen*2 + NAME_MAX*2);
	struct dirent * dp;
	QStringList filters;
	for (QString & ext : extensions)
	{
		filters.append(QString(".%1").arg(ext));
	}
	QStringList fileList = QStringList();
	std::vector<std::pair<double, QString>> mTimeNameVector;
	
	while ((dp = readdir(dirp)) != NULL)
	{
		if (dp->d_type != DT_UNKNOWN)
		{
			if (dp->d_type & DT_DIR) continue;
			if (!((dp->d_type & DT_REG) || (dp->d_type & DT_LNK))) continue;
		}
		if (fullPath.length() > fullPathLen)
		{
			fullPath.resize(fullPathLen);
		}
		fullPath.append(dp->d_name);
		if (dp->d_type == DT_UNKNOWN)
		{
			struct stat attr;
			stat(fullPath.c_str(), &attr);
			if (!(S_ISREG(attr.st_mode))) continue;
		}
		if (access(fullPath.c_str(), R_OK) != 0) continue;
		
		QString qDName = QString(dp->d_name);
		bool extensionMatch = false;
		for (QString & ext : filters)
		{
			if (qDName.endsWith(ext, Qt::CaseInsensitive))
			{
				extensionMatch = true;
				break;
			}
		}
		if (!extensionMatch) continue;
		
		if (ordering >= 2)
		{
			struct stat attr;
			stat(fullPath.c_str(), &attr);
			double t = attr.st_mtim.tv_sec + attr.st_mtim.tv_nsec * 1.0e-9;
			mTimeNameVector.push_back(std::make_pair(t, qDName));
		}
		else
		{
			fileList.append(qDName);
		}
	}
	closedir(dirp);
	
	if (ordering >= 2)
	{
		std::sort(mTimeNameVector.begin(), mTimeNameVector.end());
		for (auto &tn : mTimeNameVector)
		{
			fileList.append(tn.second);
		}
	}
	else
	{
		QCollator order;
		order.setNumericMode(true);
		std::sort(fileList.begin(), fileList.end(), order);
	}
	
	if ((ordering == 1) || (ordering == 2))
	{
		std::reverse(std::begin(fileList), std::end(fileList));
	}
	
	return fileList;
}

void MainWindow::reIndexCurrentDir(bool forced)
{
	if ((!forced) && (indexedFiles.length() > 0) && (indexedDirPath == currentDirPath)) return; // already indexed
	QApplication::setOverrideCursor(Qt::WaitCursor);
	ui.statusBar->showMessage(tr("Indexing the current directory..."));
	QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
	QElapsedTimer indexingTime;
	indexingTime.start();
	indexedDirPath = currentDirPath;
#if 0
	QDir dir(indexedDirPath);
	QStringList filters;
	for (const QString &ext : imageIO->getInputFormats())
	{
		filters.append(QString("*.%1").arg(ext));
		filters.append(QString("*.%1").arg(ext).toUpper());
	}
	QDir::SortFlags order = QDir::Name;
	switch (Globals::prefs->getFileOrder())
	{
		default:
		case 0: order = QDir::Name; break;
		case 1: order = QDir::Name | QDir::Reversed; break;
		case 2: order = QDir::Time; break;
		case 3: order = QDir::Time | QDir::Reversed; break;
	}
	indexedDir = dir.entryInfoList(filters, QDir::Files | QDir::Readable, (order | QDir::LocaleAware));
	indexedFiles = QStringList();
	for (const QFileInfo &fi : indexedDir)
	{
		indexedFiles.append(fi.fileName());
	}
#else
	indexedFiles = fastIndexer(indexedDirPath, imageIO->getInputFormats(), Globals::prefs->getFileOrder());
#endif
	qDebug() << "Found " << indexedFiles.length() << " image files in " << indexedDirPath << " within " << indexingTime.elapsed() << "ms";
	dirCountDisplay->setText(QString("%1").arg(indexedFiles.length()));
	setIndexDisplayNoSignals(indexedFiles.indexOf(currentFilePath)+1, indexedFiles.length());
	ui.statusBar->clearMessage();
	QApplication::restoreOverrideCursor();
	statusBarIndex->setText(QString("%1/%2").arg(indexDisplay->value()).arg(dirCountDisplay->text()));
}

void MainWindow::loadCurrentFile()
{
	if (slideshowDirection==0) QApplication::setOverrideCursor(Qt::WaitCursor);	// do not flicker cursor in slideshow
	QImage i = imageIO->loadFile(currentDirPath + "/" + currentFilePath);
	currentImageSize = i.size();
	display->newImage(i);
	display->setZoom(1.0);
	setImageAndWindowSize();
	QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);	// sped up drawing
	if (slideshowDirection==0) QApplication::restoreOverrideCursor();
	if (i.isNull() && (slideshowDirection==0))	// don't show warning when in slideshow
	{
		QMessageBox::critical(this, tr("Error"), QString(tr("Can not open %1")).arg(currentDirPath + "/" + currentFilePath));
	}
	
	int index = indexedFiles.indexOf(currentFilePath);
	if (thumbnailDialog)
	{
		thumbnailDialog->selectItem(index);
	}
	showFileModificationTime(currentDirPath + "/" + currentFilePath);
	setIndexDisplayNoSignals(index+1);
	setInternalState(IMAGE_FROM_FILE);
	updateDisplayOverlayIndicator();
	currentImageName = currentFilePath;
	clearUndoStack();
	statusBarIndex->setText(QString("%1/%2").arg(indexDisplay->value()).arg(dirCountDisplay->text()));
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
	if (!isFullscreen)
	{
		event->acceptProposedAction();
	}
}

void MainWindow::dragMoveEvent(QDragMoveEvent* event)
{
	if (!isFullscreen)
	{
		event->acceptProposedAction();
	}
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent* event)
{
	if (!isFullscreen)
	{
		event->accept();
	}
}

void MainWindow::dropEvent(QDropEvent* event)
{
	const QMimeData* mimeData = event->mimeData();

	if (mimeData->hasUrls())
	{
		QString fileName;
		QList<QUrl> urlList = mimeData->urls();

		if (urlList.size() > 0)
		{
			fileName = urlList.at(0).toLocalFile();
			if (!fileName.isEmpty())
			{
				QFileInfo file(fileName);
				if (file.exists())
				{
					Globals::prefs->setLastOpenedDir(file.absolutePath());
					currentFilePath = file.fileName();
					currentDirPath = file.absolutePath();
					loadCurrentFile();
					reIndexCurrentDir();
					event->acceptProposedAction();
				}
			}
		}
	}
}

void MainWindow::doSimpleFilter(SimpleFilter f)
{
	QImage src = display->getImage();
	if (src.isNull()) return; // no image
	QImage dst;
	switch(f)
	{
		case ROT_R:
			dst = src.transformed(QTransform().rotate(90.0), Qt::SmoothTransformation);
			break;
		case ROT_L:
			dst = src.transformed(QTransform().rotate(-90.0), Qt::SmoothTransformation);
			break;
		case MIRROR_V:
		#if QT_VERSION < QT_VERSION_CHECK(6,9,0)
			dst = src.mirrored(false, true);
		#else
			dst = src.flipped(Qt::Vertical);
		#endif
			break;
		case MIRROR_H:
		#if QT_VERSION < QT_VERSION_CHECK(6,9,0)
			dst = src.mirrored(true, false);
		#else
			dst = src.flipped(Qt::Horizontal);
		#endif
			break;
		case GRAYS:
			dst = src.convertToFormat(QImage::Format_Grayscale8).convertToFormat(QImage::Format_RGB32);
			if (src.hasAlphaChannel())
			{
				dst.setAlphaChannel(src.convertToFormat(QImage::Format_Alpha8));
			}
			break;
		case NEGATIVE:
			dst = src;
			dst.invertPixels();
			break;
		case COLORADJ:
			QApplication::setOverrideCursor(Qt::WaitCursor);
			dst = AutoColor::Adjust(src);
			QApplication::restoreOverrideCursor();
			break;
		case SHARPEN:
			QApplication::setOverrideCursor(Qt::WaitCursor);
			dst = Sharpener::Sharpen(src, 0.05);
			QApplication::restoreOverrideCursor();
			break;
		default:
			break;
	}
	if (!dst.isNull())
	{
		saveToUndoStack();
		display->updateImage(dst);
		currentImageSize = dst.size();
		if (src.size() != dst.size())
		{
			setImageAndWindowSize();
		}
	}
}

void MainWindow::updateDisplayOverlayIndicator()
{
	if (isFullscreen)
	{
		QString text;
		if (indexedFiles.length() > 0)
		{
			if (Globals::prefs->getFullscreenFileIndexIndicator())
			{
				text += QString("[%1/%2]").arg(indexedFiles.indexOf(currentFilePath)+1).arg(indexedFiles.length());
			}
			if (Globals::prefs->getFullscreenFileNameIndicator())
			{
				text += QString("[%1]").arg(currentImageName);
			}
		}
		displayOverlayIndicator->setText(text);
	}
}

void MainWindow::setFullscreen(bool fs)
{
	if (!(fs ^ isFullscreen)) return;
	if (fs && currentImageSize.isEmpty()) return;
	isFullscreen = fs;
	ui.menuBar->setVisible(!isFullscreen && Globals::prefs->getMenubarVisible());
	ui.toolBar->setVisible(!isFullscreen && Globals::prefs->getToolbarVisible());
	ui.statusBar->setVisible(!isFullscreen && Globals::prefs->getStatusbarVisible());
	display->enableSelection(!isFullscreen);
	if (isFullscreen)
	{
		setWindowState(windowState() | Qt::WindowFullScreen);
		display->setFrameShape(QFrame::NoFrame);
		display->setFrameShadow(QFrame::Plain);
		fullscreenZoomBefore = display->getZoom();
		setImageSize();
		QString fontFamily = Globals::prefs->getFullscreenIndicatorFontFamily();
		if (!fontFamily.isEmpty())
		{
			fontFamily = "font-family: \"" + fontFamily + "\";";
		}
		displayOverlayIndicator->setStyleSheet(QString("QLabel {color : %1; font-size: %2px; %3}").arg(Globals::prefs->getFullscreenIndicatorColor()).arg(Globals::prefs->getFullscreenIndicatorFontSize()).arg(fontFamily));
		displayOverlayIndicator->clear();
		updateDisplayOverlayIndicator();
		if (Globals::prefs->getFullscreenHideCursor())
		{
			QApplication::setOverrideCursor(Qt::BlankCursor);
		}
	}
	else
	{
		setWindowState(windowState() & ~Qt::WindowFullScreen);
		display->setFrameShape(QFrame::Panel);
		display->setFrameShadow(QFrame::Sunken);
		display->setZoom(fullscreenZoomBefore);
		setImageAndWindowSize();
		displayOverlayIndicator->clear();
		while (QApplication::overrideCursor())
		{
			QApplication::restoreOverrideCursor();
		}
	}
	applyInternalState();
}

void MainWindow::setWindowSize()
{
	if (isFullscreen) return;
	if (windowState() & Qt::WindowMaximized) return;
	if ((Globals::prefs->getDisplayMode() == 0) || (Globals::prefs->getDisplayMode() == 5))	//fit window to image & fit large images to desktop
	{
		QRect desktopRect = QApplication::primaryScreen()->availableGeometry();
		QSize windowExtra = frameGeometry().size() - geometry().size();
		QSize availableSize = desktopRect.size() - windowExtra;
		
		display->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		display->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		
		QSize oldSize = this->size();
		QSize extraSize = oldSize - display->viewport()->size();
		QSize newSize = currentImageSize * (display->getZoom() / Globals::scalingFactor) + extraSize + QSize(1,1);
		newSize = newSize.boundedTo(availableSize);
		this->resize(newSize);
		
		if (Globals::prefs->getDisplayMode() == 0) //fit window to image
		{
			display->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
			display->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
			
			if (display->horizontalScrollBar()->isVisible() || display->verticalScrollBar()->isVisible())
			{
				if (display->horizontalScrollBar()->isVisible())
				{
					newSize.setWidth(newSize.width() + display->horizontalScrollBar()->height());
				}
				if (display->verticalScrollBar()->isVisible())
				{
					newSize.setHeight(newSize.height() + display->verticalScrollBar()->width());
				}
				newSize = newSize.boundedTo(availableSize);
				this->resize(newSize);
			}
		}
	} else
	if (Globals::prefs->getDisplayMode() == 4)	//fit all images to desktop
	{
		QRect desktopRect = QApplication::primaryScreen()->availableGeometry();
		QSize windowExtra = frameGeometry().size() - geometry().size();
		QSize availableSize = desktopRect.size() - windowExtra;
		
		display->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		display->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		this->resize(availableSize);
	}
}

void MainWindow::setImageSize()
{
	if (blockSetImageSize) return;
	QRect imageRect = QRect(QPoint(), currentImageSize);
	QRect windowRect = QRect(QPoint(), display->getViewportSize());
	if (isFullscreen)
	{
		QRect screenRect = QApplication::primaryScreen()->geometry();
		QSize screenSize = QSize(screenRect.width() * Globals::scalingFactor, screenRect.height() * Globals::scalingFactor);
		switch (Globals::prefs->getFullscreenDisplayMode())
		{
			default:
			case 0: //fit all images to display
				fitImageInto(screenSize);
				break;
			case 1: //fit large images to display
				if (windowRect.contains(imageRect))
				{
					display->setZoom(1.0);
				}
				else
				{
					fitImageInto(screenSize);
				}
				break;
		}
	}
	else
	{
		switch (Globals::prefs->getDisplayMode())
		{
			case 1:	//fit all images to window
				fitImageInto(display->getViewportSize());
				break;
			case 2:	//fit large images to window
				if (windowRect.contains(imageRect))
				{
					display->setZoom(1.0);
				}
				else
				{
					fitImageInto(display->getViewportSize());
				}
				break;
			case 4:	//fit all images to desktop
				fitImageInto(display->getViewportSize());
				display->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
				display->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
				break;
			case 5:	//fit large images to desktop
				if (windowRect.contains(imageRect))
				{
					display->setZoom(1.0);
				}
				else
				{
					fitImageInto(display->getViewportSize());
				}
				display->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
				display->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
				break;
			default: break;
		}
	}
}

void MainWindow::setImageAndWindowSize()
{
	if (Globals::prefs->getDisplayMode() < 3)
	{
		setImageSize();
		setWindowSize();
	}
	else
	{
		blockSetImageSize = true;
		setWindowSize();
		blockSetImageSize = false;
		setImageSize();
	}
}

void MainWindow::fitImageInto(QSize size)
{
	double zoomW = (double)size.width() / (double)currentImageSize.width();
	double zoomH = (double)size.height() / (double)currentImageSize.height();
	if (zoomH < zoomW) zoomW = zoomH;
	display->setZoom(zoomW);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
	setImageSize();
	QMainWindow::resizeEvent(event);
}

void MainWindow::clearUndoStack()
{
	undoHistory = QList<QImage>();
	undoStackPosition = 0;
	undoIndex = 0;
	undoAction->setEnabled(false);
	redoAction->setEnabled(false);
	updateWindowTitle();
}

void MainWindow::clearPastedUndoStack()
{
	undoHistory = QList<QImage>();
	undoStackPosition = 0;
	undoIndex = 1;
	undoAction->setEnabled(false);
	redoAction->setEnabled(false);
	updateWindowTitle();
}

void MainWindow::saveToUndoStack()
{
	QImage i = display->getImage();
	while (undoStackPosition < undoHistory.length())	// clear redo steps
	{
		undoHistory.removeLast();
	}
	undoHistory.append(i);
	int minSteps = Globals::prefs->getUndoStackMinimumSteps();
	long long int memLimit = Globals::prefs->getUndoStackMemoryLimit();
	memLimit *= 1024*1024;
	while (1)
	{
		if (undoHistory.length() <= minSteps) break;
		long long int storageSize = 0;
		for (int i=0; i<undoHistory.length(); i++)
		{
			storageSize += ((long long int)undoHistory.at(i).bytesPerLine() * (long long int)undoHistory.at(i).height() + 65536);
		}
		if (storageSize < memLimit) break;
		undoHistory.removeFirst();
	}
	undoIndex += 1;
	undoStackPosition = undoHistory.length();
	undoAction->setEnabled((undoHistory.length() > 0) && (undoStackPosition > 0));
	redoAction->setEnabled(false);
	updateWindowTitle();
}

void MainWindow::undoFromUndoStack()
{
	if ((undoHistory.length() > 0) && (undoStackPosition > 0))
	{
		if (undoHistory.length() == undoStackPosition)	// first undo, save current image for redo
		{
			QImage i = display->getImage();
			undoHistory.append(i);
		}
		undoStackPosition -= 1;
		QImage i = undoHistory.at(undoStackPosition);
		display->updateImage(i);
		currentImageSize = i.size();
		setImageAndWindowSize();
		undoIndex -= 1;
	}
	undoAction->setEnabled((undoHistory.length() > 0) && (undoStackPosition > 0));
	redoAction->setEnabled((undoHistory.length() > 0) && (undoStackPosition < (undoHistory.length() - 1)));
	updateWindowTitle();
}

void MainWindow::redoFromUndoStack()
{
	if ((undoHistory.length() > 0) && (undoStackPosition < (undoHistory.length() - 1)))
	{
		undoStackPosition += 1;
		QImage i = undoHistory.at(undoStackPosition);
		display->updateImage(i);
		currentImageSize = i.size();
		setImageAndWindowSize();
		undoIndex += 1;
	}
	undoAction->setEnabled((undoHistory.length() > 0) && (undoStackPosition > 0));
	redoAction->setEnabled((undoHistory.length() > 0) && (undoStackPosition < (undoHistory.length() - 1)));
	updateWindowTitle();
}

void MainWindow::updateWindowTitle()
{
	QString title = "";
	if (!currentImageName.isEmpty())
	{
		if (undoIndex > 0) title += "* ";
		title += currentImageName;
		title += " - ";
	}
	title += "ClassicImageViewer";

	setWindowTitle(title);
}

void MainWindow::showFileModificationTime(QString fullPath)
{
	QFileInfo file(fullPath);
	QLocale locale;
	statusBarLastModified->setText(locale.toString(file.lastModified(), QLocale::ShortFormat));
}

void MainWindow::clearFileModificationTime()
{
	statusBarLastModified->setText("");
}

void MainWindow::addPathToRecentFiles(QString path)
{
	QStringList recentList = Globals::prefs->getRecentFiles();
	if (recentList.contains(path))
	{
		recentList.removeAll(path);
	}
	recentList.prepend(path);
	while (recentList.length() > 8)
	{
		recentList.removeLast();
	}
	Globals::prefs->setRecentFiles(recentList);
	updateRecentFilesMenu();
}

void MainWindow::updateRecentFilesMenu()
{
	recentFilesMenu->clear();
	for (int i=0; i<8; i++)
	{
		removeAction((Action)(ACT_RECENT_FILE_0 + i));
	}
	removeAction((Action)ACT_CLEAR_RECENT_FILES);
	QStringList list = Globals::prefs->getRecentFiles();
	for (int i=0; i<list.length(); i++)
	{
		if (i >= 8) break;
		menuAddAction(recentFilesMenu, list.at(i), (Action)(ACT_RECENT_FILE_0 + i), "",  0);
	}
	menuAddAction(recentFilesMenu, tr("&Clear recent files"), ACT_CLEAR_RECENT_FILES, "",  0);
}

void MainWindow::updateDisplayModeMenu()
{
	for (int i=0; i<6; i++)
	{
		QAction * a = searchQAction((Action)(ACT_DISPLAY_MODE_0 + i));
		a->setCheckable(true);
		a->setChecked((Globals::prefs->getDisplayMode() == i));
	}
}

QStringList MainWindow::fileDialogOpen(const QString directoryPathOverride, bool multipleFiles)
{
	
	QString baseDir = directoryPathOverride;
	if (baseDir.isEmpty())
	{
		baseDir = Globals::prefs->getLastOpenedDir();
		if (baseDir.isEmpty())
		{
			baseDir = QStandardPaths::displayName(QStandardPaths::PicturesLocation);
		}
	}
	QString filters = QString(tr("All supported images"));
	filters.append(" (");
	for ( const QString &ext : imageIO->getInputFormats())
	{
		filters.append(QString("*.%1 *.%2 ").arg(ext, ext.toUpper()));
	}
	filters.append(");;");
	
	for ( const QString &ext : imageIO->getInputFormats())
	{
		filters.append(QString("%1 files (*.%2 *.%3);;").arg(ext.toUpper(), ext, ext.toUpper()));
	}
	
	filters.append(tr("All files"));
	filters.append(" (*)");
	
	QFileDialog dialog(this, tr("Open File"), baseDir, filters);
	if (multipleFiles)
	{
		dialog.setFileMode(QFileDialog::ExistingFiles);
	}
	else
	{
		dialog.setFileMode(QFileDialog::ExistingFile);
	}
	dialog.setOptions(QFileDialog::HideNameFilterDetails);
	if (dialog.exec())
	{
		return dialog.selectedFiles();
	}
	return QStringList();
}


SaveFileDialog::ReturnCluster MainWindow::fileDialogSave(const QString directoryPathOverride, const QString defaultFileName, bool saveSelectedExtension)
{
	SaveFileDialog::ReturnCluster ret;
	ret.filePath = QString();
	ret.selectedFormat = QString();
	ret.parameters = QList<IObase::ParameterCluster>();
	
	QString baseDir = directoryPathOverride;
	if (baseDir.isEmpty())
	{
		baseDir = Globals::prefs->getLastSaveAsDir();
		if (baseDir.isEmpty())
		{
			baseDir = QStandardPaths::displayName(QStandardPaths::PicturesLocation);
		}
	}
	QString filters = QString();
	for ( const QString &ext : imageIO->getOutputFormats())
	{
		filters.append(QString("%1 files (*.%2);;").arg(ext.toUpper(), ext));
	}
	filters.chop(2);

	SaveFileDialog * dialog = new SaveFileDialog(tr("Save..."), baseDir, filters, defaultFileName, imageIO, this);
	
	if (dialog->exec() == QDialog::Accepted)
	{
		ret = dialog->getReturnCluster();
		if (saveSelectedExtension)
		{
			dialog->saveSelectedExtension();
		}
	}
	delete dialog;
	
	return ret;
}

