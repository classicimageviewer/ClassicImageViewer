// Copyright (C) 2025 zhuvoy
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
#include "iobase.h"

IObase::IObase()
{
	supportedInputFormats = QStringList();
	supportedOutputFormats = QStringList();
	//reloadConfig(); must be called in derived class constructor
}

QStringList IObase::getInputFormats()
{
	return enabledInputFormats;
}

QStringList IObase::getOutputFormats()
{
	return enabledOutputFormats;
}

QStringList IObase::getBlockedThumbnailFormats()
{
	return blockedThumbnailList;
}

QStringList IObase::getAllInputFormats()
{
	return supportedInputFormats;
}

QStringList IObase::getAllOutputFormats()
{
	return supportedOutputFormats;
}

bool IObase::tryToOpenAll()
{
	return unlistedOpen;
}

bool IObase::outputFormatSupported(QString format)
{
	return supportedOutputFormats.contains(format.toLower());
}

void IObase::addInputFormatAlternatives(QString format, QStringList alternativeList)
{
	if (supportedInputFormats.contains(format.toLower()))
	{
		for (QString alternativeFormat : alternativeList)
		{
			supportedInputFormats += alternativeFormat.toLower();
		}
		supportedInputFormats.removeDuplicates();
		supportedInputFormats.sort();
	}
}

void IObase::reloadConfig()
{
	QString moduleName = "IOmodule" + this->moduleName() + "/extensions";
	blockedOpenList = Globals::prefs->fetchSpecificParameter(moduleName, "blockedOpen", QVariant(QStringList())).toStringList();
	blockedThumbnailList = Globals::prefs->fetchSpecificParameter(moduleName, "blockedThumbnail", QVariant(QStringList())).toStringList();
	blockedSaveList = Globals::prefs->fetchSpecificParameter(moduleName, "blockedSave", QVariant(QStringList())).toStringList();
	extraOpenList = Globals::prefs->fetchSpecificParameter(moduleName, "extraOpen", QVariant(QStringList())).toStringList();
	unlistedOpen = Globals::prefs->fetchSpecificParameter(moduleName, "unlistedOpen", QVariant(false)).toBool();
	
	
	enabledInputFormats.clear();
	enabledOutputFormats.clear();
	
	for (const QString & item : supportedInputFormats)
	{
		if (!blockedOpenList.contains(item))
		{
			enabledInputFormats.append(item);
		}
	}
	enabledInputFormats += extraOpenList;
	enabledInputFormats.removeDuplicates();
	enabledInputFormats.sort();
	
	for (const QString & item : supportedOutputFormats)
	{
		if (!blockedSaveList.contains(item))
		{
			enabledOutputFormats.append(item);
		}
	}
	enabledOutputFormats.removeDuplicates();
	enabledOutputFormats.sort();

}
