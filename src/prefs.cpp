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


#include "prefs.h"
#include <QDebug>

Prefs::Prefs(QSettings * settings, QObject *parent) : QObject(parent)
{
	this->settings = settings;
	setDefault();
	if (settings)
	{
		if (settings->value("prefsMagicWord").toString() == QString("CIV1"))
		{
			#define X(xCVT,xType,xName,xDefault) valueOf ## xName = settings->value(#xName, xDefault).xCVT();
				PREFS_ENTRIES
			#undef X
		}
		else
		{
			settings->setValue("prefsMagicWord",  QString("CIV1"));
			writePrefs();
		}
	}
}

Prefs::~Prefs()
{
	writePrefs();
	if (settings)
	{
		delete settings;
	}
}

void Prefs::restoreDefaults()
{
	if (settings)
	{
		settings->clear();
	}
	setDefault();
	writePrefs();
}

void Prefs::setDefault()
{
	#define X(xCVT,xType,xName,xDefault) valueOf ## xName = xDefault;
		PREFS_ENTRIES
	#undef X
}

void Prefs::writePrefs()
{
	if (settings)
	{
		#define X(xCVT,xType,xName,xDefault) settings->setValue(#xName, valueOf ## xName);
			PREFS_ENTRIES
		#undef X
		settings->sync();
	}
}

void Prefs::storeSpecificParameter(QString unitName, QString parameterName, QVariant value)
{
	if (unitName.isEmpty()) return;
	if (parameterName.isEmpty()) return;
	if (settings)
	{
		settings->beginGroup(unitName);
		QList<QVariant> list;	// store type too
		list.append(value);
		settings->setValue(parameterName, list);
		settings->endGroup();
		settings->sync();
	}
}

QVariant Prefs::fetchSpecificParameter(QString unitName, QString parameterName, QVariant defaultValue)
{
	if (unitName.isEmpty()) return defaultValue;
	if (parameterName.isEmpty()) return defaultValue;
	QVariant value = defaultValue;
	if (settings)
	{
		settings->beginGroup(unitName);
		QList<QVariant> list = settings->value(parameterName).toList();
		if (list.length() == 1)
		{
			value = list.first();
			if (QString(value.typeName()) != QString(defaultValue.typeName()))
			{
				value = defaultValue;
			}
		}
		settings->endGroup();
	}
	return value;
}

