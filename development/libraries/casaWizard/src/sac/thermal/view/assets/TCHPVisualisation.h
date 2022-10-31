//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "ThermalMapPlotOptions.h"
#include "view/assets/SacMapsVisualisation.h"
#include "view/colormap.h"

class QPushButton;
class QComboBox;
class QCheckBox;
class QVBoxLayout;
class QLabel;

namespace casaWizard
{

class CustomCheckbox;
class Well;

namespace sac
{

namespace thermal
{

class ThermalGrid2DPlot;
class OptimizedTCHP;

class TCHPVisualisation : public SacMapsVisualisation
{
   Q_OBJECT
public:
   explicit TCHPVisualisation(QWidget *parent = nullptr);

   ThermalGrid2DPlot* TCHPPlot() const;

   void clearPlots() override;
   void updateBirdsView(const QVector<const Well*> wells, const QVector<OptimizedTCHP>& optimizedTCHPs);
   void updateSelectedWells(const QVector<int> selectedWells) override;
   void hideAllTooltips() override;
   void finalizeTooltip(const double& TCHPAtPoint, const QString& wellName);

private:
   virtual const ThermalMapPlotOptions* plotOptions() const override;

   ThermalGrid2DPlot* m_TCHPPlot;
   ThermalMapPlotOptions* m_plotOptions;
   QVBoxLayout* m_plotsAndOptions;

signals:
   void updateAvailableLayers();

public slots:
   void setColorMapType(const std::string& colorMapType) override;

private slots:
   void slotUpdateAspectRatio(int state);
   void slotUpdateColorMaps(const QString& colormapType);
   void slotUpdateWellsVisibility(int state);
};

} // namespace thermal

} // namespace sac

} // namespace casaWizard
