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
#include <QDir>
#include <QFile>

Prefs::Prefs(QString dirPath, QObject *parent) : QObject(parent)
{
	settings = NULL;
	setDefault();
	if (!dirPath.isEmpty())
	{
		settingsDirPath = dirPath;
		QDir configDir = QDir(dirPath);
		settings = new QSettings(dirPath + "/settings.cfg", QSettings::IniFormat);
		if (settings)
		{
			// migrating settings v1 to v2
			if (settings->value("prefsMagicWord").toString() == QString("CIV1"))
			{
				QFile settingsFile(configDir.absolutePath() + "/settings.cfg");
				QString backupPath = dirPath + "/backup";
				if (configDir.mkpath(backupPath))
				{
					settingsFile.copy(backupPath + "/settings.cfg");
				}
				
				for (QString group : settings->childGroups())
				{
					QSettings * groupSettings = new QSettings(settingsDirPath + "/settings_" + group + ".cfg", QSettings::IniFormat);
					settings->beginGroup(group);
					groupSettings->beginGroup(group);
					for (QString key : settings->allKeys())
					{
						groupSettings->setValue(key, settings->value(key));
					}
					groupSettings->endGroup();
					settings->endGroup();
					groupSettings->sync();
					delete groupSettings;
				}
				
				#define X(xCVT,xType,xName,xDefault) valueOf ## xName = settings->value(#xName, xDefault).xCVT();
					PREFS_ENTRIES
				#undef X
				settings->clear();
				writePrefs();
				settings->setValue("prefsMagicWord", "CIV2");
				settings->clear();
			}
			
			if (settings->value("prefsMagicWord").toString() == QString("CIV2"))
			{
				#define X(xCVT,xType,xName,xDefault) valueOf ## xName = settings->value(#xName, xDefault).xCVT();
					PREFS_ENTRIES
				#undef X
			}
			else
			{
				settings->setValue("prefsMagicWord",  QString("CIV2"));
				writePrefs();
			}
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
	for (QSettings * unitSettings : unitSettingsStorage)
	{
		unitSettings->sync();
		delete unitSettings;
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

QSettings * Prefs::getUnitSettings(QString unitName)
{
	assert(!unitName.contains(u'/'));
	if (unitSettingsStorage.contains(unitName))
	{
		return unitSettingsStorage[unitName];
	}
	if (settingsDirPath.isEmpty()) return NULL;
	QSettings * unitSettings = new QSettings(settingsDirPath + "/settings_" + unitName + ".cfg", QSettings::IniFormat);
	unitSettings->beginGroup(unitName);
	unitSettingsStorage[unitName] = unitSettings;
	return unitSettings;
}

void Prefs::storeSpecificParameter(QString unitName, QString parameterName, QVariant value)
{
	if (unitName.isEmpty()) return;
	if (parameterName.isEmpty()) return;
	QSettings * unitSettings = getUnitSettings(unitName);
	if (unitSettings)
	{
		QList<QVariant> list;	// store type too
		list.append(value);
		do {
			if (!unitSettings->contains(parameterName)) break;
			QVariant entry = unitSettings->value(parameterName);
		#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
			if (entry.type() != QVariant::List) break;
		#else
			if (entry.metaType().id() != QMetaType::QVariantList) break;
		#endif
			QList<QVariant> list = entry.toList();
			if (list.length() != 1) break;
			if (value == list.first()) return;	// unchanged value
		} while(0);
		unitSettings->setValue(parameterName, list);
		unitSettings->sync();
	}
}

QVariant Prefs::fetchSpecificParameter(QString unitName, QString parameterName, QVariant defaultValue)
{
	if (unitName.isEmpty()) return defaultValue;
	if (parameterName.isEmpty()) return defaultValue;
	QVariant value = defaultValue;
	QSettings * unitSettings = getUnitSettings(unitName);
	if (unitSettings)
	{
		QList<QVariant> list = unitSettings->value(parameterName).toList();
		if (list.length() == 1)
		{
			value = list.first();
			if (QString(value.typeName()) != QString(defaultValue.typeName()))
			{
				value = defaultValue;
			}
		}
	}
	return value;
}

void Prefs::removeSpecificParameter(QString unitName, QString parameterName)
{
	if (unitName.isEmpty()) return;
	if (parameterName.isEmpty()) return;
	QSettings * unitSettings = getUnitSettings(unitName);
	if (unitSettings)
	{
		unitSettings->remove(parameterName);
		unitSettings->sync();
	}
}

void Prefs::clearUnitSettings(QString unitName)
{
	if (unitName.isEmpty()) return;
	QSettings * unitSettings = getUnitSettings(unitName);
	if (unitSettings)
	{
		unitSettings->clear();
		unitSettings->sync();
	}
}

