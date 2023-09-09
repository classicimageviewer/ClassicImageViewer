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


#if defined(HAS_GMAGICK)
#include <Magick++.h>
#endif
#if defined(HAS_VIPS)
#include <vips.h>
#endif

#include <QApplication>
#include <QWindow>
#include <QStandardPaths>
#include <QDir>
#include <QSettings>
#include <QTranslator>
#include <QDebug>
#include <unistd.h>
#include "mainwindow.h"
#include "globals.h"
#include "civ_version.h"

#if defined(DEBUG_BUILD)
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>

void handler(int sig) {
	void * array[128];
	size_t size;

	size = backtrace(array, 128);

	fprintf(stderr, "Signal %d:\n", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(1);
}
#endif

int main(int argc, char *argv[])
{
#if defined(DEBUG_BUILD)
	signal(SIGSEGV, handler);
#endif
	
#if defined(HAS_GMAGICK)
	Magick::InitializeMagick(NULL);
#endif
#if defined(HAS_VIPS)
	if (VIPS_INIT (argv[0])) vips_error_exit (NULL);
#endif
	
	// Qt quirks
	#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	#endif

	// QApp
	QApplication app(argc, argv);
	app.setApplicationName("ClassicImageViewer");
	app.setApplicationVersion(CIV_VERSION);
	
	// Prefs
	QSettings * settings = NULL;
	QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
	if (!configPath.isEmpty())
	{
		QDir configDir = QDir(configPath + "/ClassicImageViewer");
		if (configDir.mkpath(configDir.absolutePath()))
		{
			qDebug() << "QSettings configDir: " << configDir.absolutePath();
			settings = new QSettings(configDir.absolutePath() + "/settings.cfg", QSettings::IniFormat);
		}
	}
	Globals::prefs = new Prefs(settings);

	// Locale
	QString defaultLocale = QLocale::system().name(); // e.g. "de_DE"
	defaultLocale.truncate(defaultLocale.lastIndexOf('_')); // e.g. "de"
	QString userLocale = Globals::prefs->getUserLocale();
	if (userLocale == "")
	{
		userLocale = defaultLocale;
	}
	QTranslator translator;
	bool translatorLoaded = false;
	app.installTranslator(&translator);
	QStringList qmPaths = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
	qmPaths.append("..");	// for local testing
	for(int i=0; i<qmPaths.length(); i++)
	{
		QFileInfo qmFile(qmPaths[i] + "/i18n/" + userLocale + ".qm");
		if (qmFile.exists() && qmFile.isFile())
		{
			translatorLoaded = translator.load(userLocale, qmPaths[i] + "/i18n");
			if (translatorLoaded)
			{
				qDebug() << "QTranslator locale: " << userLocale;
				break;
			}
		}
	}
	if (!translatorLoaded)
	{
		qDebug() << "QTranslator failed to load locale: " << userLocale;
	}
	
	// UI
	MainWindow mw;
	Globals::MainWindowWidget = &mw;
	mw.show();
	Globals::scalingFactor = mw.windowHandle()->devicePixelRatio();
	
	// open image file pass as argument
	if(argc > 1)
	{
		mw.setFileToBeOpenedOnStartup(QString(argv[1]));
	}

	// Start App
	int ret = app.exec();
	
	// Conclude
	delete Globals::prefs;
	qDebug() << "Normal exit";
	return ret;
}
