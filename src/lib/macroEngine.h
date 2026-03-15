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


#ifndef MACROENGINE_H
#define MACROENGINE_H

#include <QObject>
#include <QImage>
#include "effects/effectBase.h"
#include "effects/effectHub.h"

class MacroEngine : public QObject
{
	Q_OBJECT
private:
	EffectHub * effectHub;
public:
	using QObject::QObject;
	
	enum class Op : int {
		RotateLeft,
		RotateRight,
		RotateCustom,
		FlipVertical,
		FlipHorizontal,
		Shear,
		Resize,
		AddBorder,
		PaddToSize,
		ColorDepthIncrease,
		ColorDepthDecrease,
		Grayscale,
		Negative,
		AdjustColors,
		AutoColorAdjust,
		Sharpen,
		Effects,
		ExternalTools
	};
	
	MacroEngine();
	~MacroEngine();
	QImage runMacro(QImage image, QList<QMap<QString, QVariant>> macro);
};

#endif //MACROENGINE_H
