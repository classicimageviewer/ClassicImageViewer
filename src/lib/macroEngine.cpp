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


#include "macroEngine.h"
#include <omp.h>
#include "globals.h"
#include "imageOp.h"
#include "autocolor.h"
#include "resizer.h"
#include <cmath>
#include <QDebug>

MacroEngine::MacroEngine()
{
	effectHub = new EffectHub();
}

MacroEngine::~MacroEngine()
{
	delete effectHub;
}

QImage MacroEngine::runMacro(QImage image, QList<QMap<QString, QVariant>> macro)
{
	for (const QMap<QString, QVariant> & step : macro)
	{
		//qDebug() << "Executing: " << step;
		MacroEngine::Op op = static_cast<MacroEngine::Op>(step["opCode"].toInt());
		
		switch (op)
		{
			default:
				assert(0);
				break;
			case MacroEngine::Op::RotateLeft:
				image = ImageOp::RotateLeft(image);
				break;
			case MacroEngine::Op::RotateRight:
				image = ImageOp::RotateRight(image);
				break;
			//case MacroEngine::Op::RotateCustom:
			//	break;
			case MacroEngine::Op::FlipVertical:
				image = ImageOp::MirrorVertical(image);
				break;
			case MacroEngine::Op::FlipHorizontal:
				image = ImageOp::MirrorHorizontal(image);
				break;
			//case MacroEngine::Op::Shear:
			//	break;
			case MacroEngine::Op::Resize:
				{
					if (!step.contains("opConfig")) return QImage();
					QMap<QString, QVariant> opConfig = step["opConfig"].toMap();
					QSize newSize;
					int resizeMode = opConfig["resizeMode"].toInt();
					int resizeAlgorithm = opConfig["resizeAlgorithm"].toInt();
					int w = opConfig["resizeW"].toDouble();
					int h = opConfig["resizeH"].toDouble();
					double percent = opConfig["resizeW"].toDouble();
					double aspectRatio = (double)image.width() / (double)image.height();
					
					switch (resizeMode)
					{
						case 0:	// width
							newSize = QSize(w, std::round(w/aspectRatio));
							break;
						case 1:	// height
							newSize = QSize(std::round(w*aspectRatio), w);
							break;
						case 2:	// long side
							if (image.width() > image.height())
							{
								newSize = QSize(w, std::round(w/aspectRatio));
							}
							else
							{
								newSize = QSize(std::round(w*aspectRatio), w);
							}
							break;
						case 3:	// short side
							if (image.width() < image.height())
							{
								newSize = QSize(w, std::round(w/aspectRatio));
							}
							else
							{
								newSize = QSize(std::round(w*aspectRatio), w);
							}
							break;
						case 4:	// scale to
							newSize = image.size() * (percent / 100.0);
							break;
						case 5:	// new size
							newSize = QSize(w, h);
							break;
					}
					
					if (newSize.isEmpty()) return QImage();
					image = Resizer::Resize(image, newSize, static_cast<Resizer::Algorithm>(resizeAlgorithm));
				}
				break;
			case MacroEngine::Op::AddBorder:
				{
					if (!step.contains("opConfig")) return QImage();
					QMap<QString, QVariant> opConfig = step["opConfig"].toMap();
					
					QColor backgroundColor = opConfig["backgroundColor"].value<QColor>();
					int top = opConfig["top"].toInt();
					int right = opConfig["right"].toInt();
					int bottom = opConfig["bottom"].toInt();
					int left = opConfig["left"].toInt();
					
					QImage dst(image.size()+QSize(left+right, top+bottom), image.format());
					dst.fill(backgroundColor);
					QPainter p(&dst);
					p.drawImage(left, top, image);
					p.end();
					image = dst;
				}
				break;
			case MacroEngine::Op::PaddToSize:
				{
					if (!step.contains("opConfig")) return QImage();
					QMap<QString, QVariant> opConfig = step["opConfig"].toMap();
					
					QColor backgroundColor = opConfig["backgroundColor"].value<QColor>();
					int width = opConfig["width"].toInt();
					int height = opConfig["height"].toInt();
					double widthRatio = opConfig["widthRatio"].toDouble();
					double heightRatio = opConfig["heightRatio"].toDouble();
					int mode = opConfig["mode"].toInt();
					int method = opConfig["method"].toInt();
					
					if (method == 1)
					{
						width = image.width();
						height = image.height();
						if ((width == 0) && (height == 0)) return QImage();
						double sourceAR = (double)width / (double)height;
						double destinationAR = widthRatio / heightRatio;
						if (destinationAR > sourceAR)
						{
							width = std::round(height * destinationAR);
						}
						else
						{
							height = std::round(width / destinationAR);
						}
					}
					
					if ((width == 0) && (height == 0)) return QImage();
					
					QImage dst(QSize(width, height), image.format());
					dst.fill(backgroundColor);
					QPainter p(&dst);
					switch (mode)
					{
						default:
						case 0:
							p.drawImage((width - image.width())/2, (height - image.height())/2, image);
							break;
						case 1:
							p.drawImage(width - image.width(), height - image.height(), image);
							break;
						case 2:
							p.drawImage(0, height - image.height(), image);
							break;
						case 3:
							p.drawImage(width - image.width(), 0, image);
							break;
						case 4:
							p.drawImage(0, 0, image);
							break;
					}
					p.end();
					image = dst;
				}
				break;
			case MacroEngine::Op::ColorDepthIncrease:
				{
					if (image.hasAlphaChannel())
					{
						image = image.convertToFormat(QImage::Format_ARGB32).copy();
					}
					else
					{
						image = image.convertToFormat(QImage::Format_RGB32).copy();
					}
				}
				break;
			case MacroEngine::Op::Grayscale:
				image = ImageOp::Grayscale(image);
				break;
			case MacroEngine::Op::Negative:
				image = ImageOp::Negative(image);
				break;
			case MacroEngine::Op::AutoColorAdjust:
				image = AutoColor::Adjust(image);
				break;
			case MacroEngine::Op::Sharpen:
				image = ImageOp::Sharpen(image, 0.05);
				break;
			case MacroEngine::Op::ColorDepthDecrease:		// these are all effects
			case MacroEngine::Op::AdjustColors:
			case MacroEngine::Op::Effects:
				{
					if (!step.contains("opConfig")) return QImage();
					QMap<QString, QVariant> opConfig = step["opConfig"].toMap();
					
					int id = opConfig["effectId"].toInt();
					QList<EffectBase::ParameterCluster> parameterList;
					
					if (!opConfig.contains("effectParamsControlType")) return QImage();
					if (!opConfig.contains("effectParamsParameterName")) return QImage();
					if (!opConfig.contains("effectParamsParameterValue")) return QImage();
					
					QStringList effectParamsControlType = opConfig["effectParamsControlType"].toStringList();
					QStringList effectParamsParameterName = opConfig["effectParamsParameterName"].toStringList();
					QList<QVariant> effectParamsParameterValue = opConfig["effectParamsParameterValue"].toList();
					
					int paramsLen = effectParamsParameterName.length();
					if (effectParamsControlType.length() != paramsLen) return QImage();
					if (effectParamsParameterValue.length() != paramsLen) return QImage();
					
					for (int i = 0; i < paramsLen; i++)
					{
						EffectBase::ParameterCluster param;
						param.controlType = effectParamsControlType[i];
						param.parameterName = effectParamsParameterName[i];
						param.parameterValue = effectParamsParameterValue[i];
						parameterList.append(param);
					}
					
					image = effectHub->applyEffect(id, image, parameterList);
				}
				break;
			//case MacroEngine::Op::ExternalTools:
			//	break;
		}
	}
	return image;
}
