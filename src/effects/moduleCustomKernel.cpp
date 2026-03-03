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

#include "moduleCustomKernel.h"
#include "globals.h"
#include <QDebug>
#include "lib/imageOp.h"


EffectModuleCustomKernel::EffectModuleCustomKernel(QObject * parent) : QObject(parent)
{
	setModuleName("EffectModuleCustomKernel");
}

EffectModuleCustomKernel::~EffectModuleCustomKernel()
{
	
}

QString EffectModuleCustomKernel::getDefaultKernelString()
{
	QString defaultText = QString(tr("# comma or space separated integer values"));
	defaultText.append("\n0, 0, 0");
	defaultText.append("\n0, 1, 0");
	defaultText.append("\n0, 0, 0");
	return defaultText;
}

QList<EffectBase::ParameterCluster> EffectModuleCustomKernel::getListOfParameterClusters()
{
	QList<EffectBase::ParameterCluster> cluster;
	
	cluster += uiParamTextEdit(tr("Kernel"), "Kernel", getDefaultKernelString());
	cluster += uiParamSpinbox(tr("Divisor"),"Divisor", 1, 1, 999999);
	cluster += uiParamSpinbox(tr("Offset"),"Offset", 0, -256, 256);
	
	QStringList list = QStringList();
	list.append(QString(tr("Mirror")));
	list.append(QString(tr("Extend")));  
	cluster += uiParamCombobox(tr("Edge"), "Edge", 0, list);
	
	return cluster;
}

QImage EffectModuleCustomKernel::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	QString kernelString = getParamStringValue(parameters, "Kernel", getDefaultKernelString());
	int div = getParamIntValue(parameters, "Divisor", 1);
	int offset = getParamIntValue(parameters, "Offset", 0);
	int edge = getParamIntValue(parameters, "Edge", 0);
	
	QStringList lines = kernelString.split("\n");
	QStringList processedKernelString;
	int kernelMaxWidth = 0;
	for (QString & line : lines)
	{
		line = line.split("#").at(0);
		line = line.replace(',', " ");
		line = line.replace('\t', " ");
		line = line.trimmed();
		while (line.contains("  "))
		{
			line = line.replace("  ", " ");
		}
		if (line.isEmpty()) continue;
		if (line == " ") continue;
		QStringList elements = line.split(" ");
		QString processedLine;
		int widthCount = 0;
		for (QString & element : elements)
		{
			QString e = QString("%1").arg(element.toInt());
			if (!processedLine.isEmpty())
			{
				processedLine.append(" ");
			}
			processedLine.append(e);
			widthCount += 1;
		}
		if (!processedLine.isEmpty())
		{
			processedKernelString.append(processedLine);
			if (kernelMaxWidth < widthCount)
			{
				kernelMaxWidth = widthCount;
			}
		}
	}
	
	QSize kernelSize = QSize(((kernelMaxWidth/2)*2)+1, ((processedKernelString.size()/2)*2)+1);
	int * kernel =new int[kernelSize.width() * kernelSize.height()]{0};
	for (int y = 0; y < processedKernelString.size(); y++)
	{
		int x = 0;
		QStringList line = processedKernelString.at(y).split(" ");
		for (QString & element : line)
		{
			kernel[y*kernelMaxWidth + x] = element.toInt();
			x += 1;
		}
	}
	
	ImageOp::EdgeHandling edgeHandling;
	switch(edge)
	{
		default:
		case 0:
			edgeHandling = ImageOp::EdgeHandling::Mirror;
			break;
		case 1:
			edgeHandling = ImageOp::EdgeHandling::Extend;
			break;
	}
	
	QImage processedImage = ImageOp::Kernel2D(image, edgeHandling, kernelSize, reinterpret_cast<const int *>(kernel), div, offset);
	delete [] kernel;
	return processedImage;
}

