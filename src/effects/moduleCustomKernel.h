// Copyright (C) 2026 zhuvoy
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

#ifndef MODULECUSTOMKERNEL_H
#define MODULECUSTOMKERNEL_H

#include "effectBase.h"

#include <QObject>
#include <QVariant>
#include <QImage>
#include <QList>
#include <QStringList>

class EffectModuleCustomKernel : public QObject, public EffectBase
{
	Q_OBJECT
private:
	QString getDefaultKernelString();
public:
	EffectModuleCustomKernel(QObject * parent = NULL);
	~EffectModuleCustomKernel() override;
	
	bool available() override {return true;};
	QString getName() override  {return QString(tr("Custom kernel"));};
	bool previewModeIsZoom() override  {return true;};
	
	QList<EffectBase::ParameterCluster> getListOfParameterClusters() override;
	QImage applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters) override;
};

#endif //MODULECUSTOMKERNEL_H
