//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QWidget>
#include <memory>
#include "view/colormap.h"

class QCheckBox;
class QGridLayout;

namespace casaWizard
{

namespace sac
{
class MapPlotOptions;

class MapsVisualisation : public QWidget
{
   Q_OBJECT
public:
   explicit MapsVisualisation(QWidget *parent = nullptr);
   virtual ~MapsVisualisation() = default;

   virtual void clearPlots() = 0;
   virtual void updateSelectedWells(const QVector<int> selectedWells) = 0;
   virtual void hideAllTooltips() = 0;

   QCheckBox* wellsVisible() const;

public slots:
   virtual  void setColorMapType(const std::string& colorMapType) = 0;

protected:
   virtual const MapPlotOptions* plotOptions() const= 0;
   ColorMap& colormap();
   const ColorMap& colormap() const;

private:
   std::unique_ptr<ColorMap> colormap_;
};

} //sac
} //casaWizard
