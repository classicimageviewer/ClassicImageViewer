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


#ifndef EFFECTHUB_H
#define EFFECTHUB_H

#include <QObject>
#include <QImage>
#include <QList>
#include "effects/effectbase.h"

class EffectHub : public QObject
{
	Q_OBJECT
private: // variables
	QList<EffectBase*> modules;
	QStringList moduleNames;
private: // functions
	bool checkId(int effectId);
public:
	EffectHub(QObject * parent = NULL);
	~EffectHub();
	
	QStringList getEffects();
	
	bool previewModeIsZoom(int effectId);
	QList<EffectBase::ParameterCluster> getListOfParameterClusters(int effectId);
	void saveEffectParameters(int effectId, QList<EffectBase::ParameterCluster> parameters);
	QImage applyEffect(int effectId, QImage image, QList<EffectBase::ParameterCluster> parameters);
};

#endif //EFFECTHUB_H
