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

class SacMapPlotOptions;

class SacMapsVisualisation : public QWidget
{
   Q_OBJECT
public:
   explicit SacMapsVisualisation(QWidget *parent = nullptr);
   virtual ~SacMapsVisualisation() = default;

   virtual void clearPlots() = 0;
   virtual void updateSelectedWells(const QVector<int> selectedWells) = 0;
   virtual void hideAllTooltips() = 0;

   QCheckBox* wellsVisible() const;
   const ColorMap& colormap() const;
   ColorMap& colormap();

public slots:
   virtual  void setColorMapType(const std::string& colorMapType) = 0;

protected:
   virtual const SacMapPlotOptions* plotOptions() const= 0;

private:
   std::unique_ptr<ColorMap> colormap_;
};

} // namespace sac

} // namespace casaWizard
