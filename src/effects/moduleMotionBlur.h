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

#ifndef MODULEMOTIONBLUR_H
#define MODULEMOTIONBLUR_H

#include "effectBase.h"

#include <QObject>
#include <QVariant>
#include <QImage>
#include <QList>
#include <QStringList>

class EffectModuleMotionBlur : public QObject, public EffectBase
{
	Q_OBJECT
private:
	struct intRGBA_t
	{
		int r;
		int g;
		int b;
		int a;
		intRGBA_t() : r(0), g(0), b(0), a(0) {}
		intRGBA_t(int r, int g, int b, int a) : r(r), g(g), b(b), a(a) {}
		intRGBA_t& operator+=(const intRGBA_t& other)
		{
			r += other.r;
			g += other.g;
			b += other.b;
			a += other.a;
			return *this;
		}
		intRGBA_t& operator/=(int div)
		{
			r /= div;
			g /= div;
			b /= div;
			a /= div;
			return *this;
		}
	};
	intRGBA_t interpolatedPixel(const QRgb ** rows, int w, int h, float x, float y);
public:
	EffectModuleMotionBlur(QObject * parent = NULL);
	~EffectModuleMotionBlur() override;
	
	bool available() override {return true;};
	QString getName() override  {return QString(tr("Motion blur"));};
	bool previewModeIsZoom() override  {return false;};
	
	QList<EffectBase::ParameterCluster> getListOfParameterClusters() override;
	QImage applyEffect(QImage image, QList<EffectBase::ParameterCluster> parameters) override;
};

#endif //MODULEMOTIONBLUR_H
