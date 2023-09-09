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


#include "savefiledialog.h"
#include "globals.h"
#include <QDebug>
#include <QMessageBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>

SaveFileDialog::SaveFileDialog(const QString &caption, const QString &directory, const QString &filter, const QString &fileName, ImageIO * imageIO, QWidget *parent) : QFileDialog(parent)
{
	setOption(QFileDialog::DontUseNativeDialog, true);
	setAcceptMode(QFileDialog::AcceptSave);
	setWindowTitle(caption);
	setDirectory(directory);
	setNameFilter(filter);
	selectFile(fileName);
	this->imageIO = imageIO;
	
	QFileInfo info = QFileInfo(fileName);
	baseName = info.completeBaseName();
	defaultExtension = info.suffix();
	if (defaultExtension.isEmpty()) defaultExtension = Globals::prefs->fetchSpecificParameter("SaveFileDialog", "defaultExtension", QString("jpg")).toString();

	QGridLayout * layout = static_cast<QGridLayout*>(this->layout());
	QHBoxLayout * sidePanel = new QHBoxLayout();
	layout->addLayout(sidePanel, 0, layout->columnCount(), -1, 1);
	QVBoxLayout * sideLine = new QVBoxLayout();
	QFrame * line = new QFrame();
	line->setFrameShape(QFrame::VLine);
	line->setFrameShadow(QFrame::Sunken);
	sideLine->addWidget(line);
	sidePanel->addLayout(sideLine);
	sidekickLayout = new QVBoxLayout();
	sidePanel->addLayout(sidekickLayout);

	QRegularExpression regex(QString("\\(\\*.(\\w+)"));
	for (const QString & entry : nameFilters())
	{
		QRegularExpressionMatch match = regex.match(entry);
		if (match.hasMatch())
		{
			if (defaultExtension.compare(match.captured(1), Qt::CaseInsensitive) == 0)
			{
				setDefaultSuffix(match.captured(1));
				selectNameFilter(entry);
				filterChanged(entry);
				break;
			}
			
		}
	}
	
	connect(this, SIGNAL(filterSelected(QString)), this, SLOT(filterChanged(QString)));
}

SaveFileDialog::~SaveFileDialog()
{
	deleteSideKick();
}

void SaveFileDialog::deleteSideKick()
{
	QLayoutItem * child;
	while ((child = sidekickLayout->takeAt(0)) != NULL)
	{
		delete child->widget();
		delete child;
	}
}

void SaveFileDialog::filterChanged(const QString &filter)
{
	Q_UNUSED(filter);
	deleteSideKick();
	QRegularExpression regex(QString("\\(\\*.(\\w+)"));
	QRegularExpressionMatch match = regex.match(selectedNameFilter());
	if (match.hasMatch())
	{
		setDefaultSuffix(match.captured(1));
		selectFile(baseName + "." + match.captured(1));
		QList<IObase::ParameterCluster> parameterList = imageIO->getListOfParameterClusters(match.captured(1));
		if (parameterList.length() > 0)
		{
			for (IObase::ParameterCluster elem : parameterList)
			{
				if (elem.controlType == "spinbox")
				{
					sidekickLayout->addWidget(new QLabel(elem.displayName));
					QSpinBox * sb = new QSpinBox();
					sb->setObjectName(elem.parameterName);
					sb->setMinimum(elem.parameterMinValue.toInt());
					sb->setMaximum(elem.parameterMaxValue.toInt());
					sb->setValue(elem.parameterValue.toInt());
					sidekickLayout->addWidget(sb);
				} else
				if (elem.controlType == "doublespinbox")
				{
					sidekickLayout->addWidget(new QLabel(elem.displayName));
					QDoubleSpinBox * sb = new QDoubleSpinBox();
					sb->setObjectName(elem.parameterName);
					sb->setMinimum(elem.parameterMinValue.toDouble());
					sb->setMaximum(elem.parameterMaxValue.toDouble());
					sb->setValue(elem.parameterValue.toDouble());
					sidekickLayout->addWidget(sb);
				} else
				if (elem.controlType == "checkbox")
				{
					QCheckBox * cb = new QCheckBox(elem.displayName);
					cb->setObjectName(elem.parameterName);
					cb->setChecked(elem.parameterValue.toBool());
					sidekickLayout->addWidget(cb);
				} else
				if (elem.controlType == "combobox")
				{
					sidekickLayout->addWidget(new QLabel(elem.displayName));
					QComboBox * cb = new QComboBox();
					cb->setObjectName(elem.parameterName);
					cb->addItems(elem.parameterMinValue.toStringList());
					cb->setCurrentIndex(elem.parameterValue.toInt());
					sidekickLayout->addWidget(cb);
				}
				//TODO more
			}
			sidekickLayout->addItem(new QSpacerItem(8,8, QSizePolicy::Minimum, QSizePolicy::Expanding));
		}
		if (sidekickLayout->count() > 0)
		{
			QCheckBox * cb = new QCheckBox(tr("Save these settings"));
			cb->setObjectName("SaveFileDialogSaveParameters");
			cb->setChecked(false);
			sidekickLayout->addWidget(cb);
		}
	}
}

SaveFileDialog::ReturnCluster SaveFileDialog::getReturnCluster()
{
	ReturnCluster ret;
	ret.filePath = QString();
	ret.selectedFormat = QString();
	ret.parameters = QList<IObase::ParameterCluster>();
	
	QStringList list = selectedFiles();
	if (list.length() > 0)
	{
		ret.filePath = list.at(0);
	}
	ret.selectedFormat = defaultSuffix();

	bool saveParameters = false;
	QList<IObase::ParameterCluster> parameterList = imageIO->getListOfParameterClusters(defaultSuffix());
	for (int i=0; i<sidekickLayout->count(); i++)
	{
		QLayoutItem * child = sidekickLayout->itemAt(i);
		if (child != NULL)
		{
			QWidget * widget = child->widget();
			if (widget != NULL)
			{
				if (widget->objectName() == "SaveFileDialogSaveParameters")
				{
					saveParameters = ((QCheckBox*)widget)->isChecked();
					continue;
				}
				for (IObase::ParameterCluster elem : parameterList)
				{
					if ((elem.parameterName.length() > 0) && (elem.parameterName == widget->objectName()))
					{
						IObase::ParameterCluster cluster;
						cluster.parameterName = elem.parameterName;
						if (elem.controlType == "spinbox")
						{
							cluster.parameterValue = QVariant(((QSpinBox*)widget)->value());
						} else
						if (elem.controlType == "doublespinbox")
						{
							cluster.parameterValue = QVariant(((QDoubleSpinBox*)widget)->value());
						} else
						if (elem.controlType == "checkbox")
						{
							cluster.parameterValue = QVariant(((QCheckBox*)widget)->isChecked());
						} else
						if (elem.controlType == "combobox")
						{
							cluster.parameterValue = QVariant(((QComboBox*)widget)->currentIndex());
						} else
						//TODO more
						{
							break;
						}
						
						ret.parameters.append(cluster);
						break;
					}
				}
			}
		}
	}
	if ((ret.parameters.length() > 0) && saveParameters)
	{
		IObase::ParameterCluster cluster;
		cluster.parameterName = "SaveFileDialogSaveParameters";
		cluster.parameterValue = QVariant(true);
		ret.parameters.prepend(cluster);	// must be the first element
	}
	
	return ret;
}

void SaveFileDialog::saveSelectedExtension()
{
	Globals::prefs->storeSpecificParameter("SaveFileDialog", "defaultExtension", defaultSuffix());
}

