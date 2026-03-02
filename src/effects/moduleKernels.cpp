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

#include "moduleKernels.h"
#include "globals.h"
#include <QDebug>
#include "lib/imageOp.h"


EffectModuleKernels::EffectModuleKernels(QObject * parent) : QObject(parent)
{
	setModuleName("EffectModuleKernels");
}

EffectModuleKernels::~EffectModuleKernels()
{
	
}

QList<EffectBase::ParameterCluster> EffectModuleKernels::getListOfParameterClusters()
{
	QList<EffectBase::ParameterCluster> cluster;

	QStringList list = QStringList();
	list.append(QString(tr("Emboss")));
	list.append(QString(tr("Emboss 2")));
	list.append(QString(tr("Outline")));
	list.append(QString(tr("Edge")));
	list.append(QString(tr("Relief")));
	list.append(QString(tr("Relief 2")));
	list.append(QString(tr("Smooth")));
	list.append(QString(tr("Detail")));
	list.append(QString(tr("Focus")));
	list.append(QString(tr("Edge enhance")));
	list.append(QString(tr("Highpass")));
	cluster += uiParamCombobox(tr("Kernel"), "Kernel", 0, list);
	
	return cluster;
}

QImage EffectModuleKernels::applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters)
{
	int kernelSelect = getParamIntValue(parameters, "Kernel", 0);
	
	const int * kernel = NULL;
	QSize kernelSize = QSize(3, 3);
	int div = 1;
	int offset = 0;
	
	switch (kernelSelect)
	{
		default: kernel =  new int[3*3]{		// identity
					0, 0, 0,
					0, 1, 0,
					0, 0, 0 };
			break;
		case 0: kernel = new int[3*3]{			// emboss
					-1, -1,  0,
					-1,  1,  1,
					 0,  1,  1 };
			break;
		case 1: kernel = new int[3*3]{			// emboss2
					-2, -1,  0,
					-1,  1,  1,
					 0,  1,  2 };
			break;
		case 2: kernel = new int[3*3]{			// outline
					-1, -1, -1,
					-1,  8, -1,
					-1, -1, -1 };
			break;
		case 3: kernel = new int[3*3]{			// edge
					-2, -1,  0,
					-1,  0,  1,
					 0,  1,  2 };
			break;
		case 4: kernel = new int[3*3]{			// relief;
					-1, -1,  0,
					-1,  0,  1,
					 0,  1,  1 };
			offset = 127;
			break;
		case 5: kernel = new int[3*3]{			// relief2
					-2, -1,  0,
					-1,  0,  1,
					 0,  1,  2 };
			offset = 127;
			break;
		case 6: kernel = new int[3*3]{			// smooth
					1, 1, 1,
					1, 5, 1,
					1, 1, 1 };
			div = 13;
			break;
		case 7: kernel = new int[3*3]{			// detail
					 0, -1,  0,
					-1, 10, -1,
					 0, -1,  0 };
			div = 6;
			break;
		case 8: kernel = new int[3*3]{			// focus
					-1, 0, -1,
					 0, 7,  0,
					-1, 0, -1 };
			div = 3;
			break;
		case 9: kernel = new int[3*3]{			// edge enhance
					-1, -1, -1,
					-1, 10, -1,
					-1, -1, -1 };
			div = 2;
			break;
		case 10: kernel = new int[3*3]{			// highpass
					-1, -2, -1,
					-2, 12, -2,
					-1, -2, -1 };
			div = 16;
			offset = 127;
			break;
	}
	QImage processedImage = ImageOp::Kernel2D(image, ImageOp::EdgeHandling::Mirror, kernelSize, kernel, div, offset);
	delete [] kernel;
	return processedImage;
}

