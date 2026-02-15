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


#include "globals.h"
#include <QThread>

// one instance
Globals globalsInstance;

// define static members
Prefs * Globals::prefs = NULL;
QMainWindow * Globals::MainWindowWidget = NULL;
double Globals::scalingFactor = 1.0;

int Globals::getThreadCount(void)
{
	if (Globals::prefs == NULL) return 1;
	
	int threadCount = Globals::prefs->getInternalThreads();
	if (threadCount == 0)
	{
		threadCount = QThread::idealThreadCount();
	}
	else
	{
		int exp = threadCount - 1;
		threadCount = 1;
		for (int i=0; i<exp; i++ )
		{
			threadCount *= 2;
		}
	}
	if (threadCount < 1) threadCount = 1;
	return threadCount;
}
