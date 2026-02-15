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


#ifndef PREFS_H
#define PREFS_H

#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QPoint>

// important/global preferences
#define PREFS_ENTRIES \
	/*X(QVCVT, TYPE, NAME, DEFAULT_VALUE)*/ \
	X(toString, QString, UserLocale, "") \
	X(toBool, bool, MaximizedWindow, false) \
	X(toBool, bool, StartFullscreen, false) \
	X(toPoint, QPoint, WindowPosition, QPoint(0,0)) \
	X(toBool, bool, MenubarVisible, true) \
	X(toBool, bool, ToolbarVisible, true) \
	X(toBool, bool, StatusbarVisible, true) \
	X(toString, QString, LastOpenedDir, "") \
	X(toString, QString, LastSaveAsDir, "") \
	X(toStringList, QStringList, RecentFiles, QStringList()) \
	X(toStringList, QStringList, ExternalEditors, QStringList()) \
	X(toInt, int, FileOrder, 0) \
	X(toBool, bool, LoopDir, true) \
	X(toInt, int, DisplayMode, 0) \
	X(toBool, bool, FitWindowWhenZoomed, true) \
	X(toInt, int, DisplayBackground, 0) \
	X(toInt, int, ZoomDelta, 10) \
	X(toBool, bool, ReverseWheel, false) \
	X(toBool, bool, ClearClipboardOnExit, true) \
	X(toBool, bool, ClearRecentFilesOnExit, false) \
	X(toBool, bool, UseFastSelector, false) \
	X(toBool, bool, EnableToolbarShrinking, false) \
	X(toBool, bool, DisplayHigherQuality, false) \
	X(toBool, bool, FullscreenHideCursor, true) \
	X(toInt, int, FullscreenDisplayMode, 0) \
	X(toBool, bool, FullscreenFileIndexIndicator, true) \
	X(toBool, bool, FullscreenFileNameIndicator, false) \
	X(toString, QString, FullscreenIndicatorColor, "green") \
	X(toString, QString, FullscreenIndicatorFontFamily, "") \
	X(toInt, int, FullscreenIndicatorFontSize, 8) \
	X(toInt, int, UndoStackMinimumSteps, 4) \
	X(toInt, int, UndoStackMemoryLimit, 512) \
	X(toInt, int, ThumbnailsCacheSize, 512) \
	X(toBool, bool, ThumbnailsPreloading, true) \
	X(toInt, int, ThumbnailsThreads, 0) \
	X(toInt, int, ThumbnailsScrollSpeed, 32) \
	X(toInt, int, InternalThreads, 0) \
	// end_of_list


class Prefs : public QObject
{
	Q_OBJECT
private:
	QSettings * settings;
	void setDefault();
	void writePrefs();
	
	// local prefs storage
	#define X(xCVT,xType,xName,xDefault) xType valueOf ## xName;
		PREFS_ENTRIES
	#undef X
public:
	Prefs(QSettings * settings, QObject *parent = NULL);
	~Prefs();
	void restoreDefaults();

	// prefs getters
	#define X(xCVT,xType,xName,xDefault) xType get ## xName() {return valueOf ## xName;}
		PREFS_ENTRIES
	#undef X
	// prefs setters
	#define X(xCVT,xType,xName,xDefault) void set ## xName(const xType newValue) {valueOf ## xName = newValue; if (settings) {settings->setValue(#xName, newValue);}}
		PREFS_ENTRIES
	#undef X
	
	void storeSpecificParameter(QString unitName, QString parameterName, QVariant value);
	QVariant fetchSpecificParameter(QString unitName, QString parameterName, QVariant defaultValue = QVariant());
	void removeSpecificParameter(QString unitName, QString parameterName);
};

#endif //PREFS_H
