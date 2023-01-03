//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "MapPlotOptionsLithofraction.h"
#include "view/assets/SacMapsVisualisation.h"
#include "view/colormap.h"

class QComboBox;
class QGridLayout;
class QLabel;

namespace casaWizard
{

class CustomCheckbox;
class Well;

namespace sac
{
class LithoGrid2DPlot;
class OptimizedLithofraction;

class LithofractionVisualisation : public SacMapsVisualisation
{
   Q_OBJECT
public:
   explicit LithofractionVisualisation(QWidget *parent = nullptr);

   std::vector<LithoGrid2DPlot*> lithoFractionPlots() const;

   const QComboBox* layerSelection() const;
   QComboBox* lithotypeSelection() const;
   CustomCheckbox* singleMapLayout() const;

   void clearPlots() override;
   void updateBirdsView(const QVector<const Well*> wells, const QVector<OptimizedLithofraction>& optimizedLithoFractions);
   void updateLayerOptions(QStringList availableLayers);
   void updateSelectedWells(const QVector<int> selectedWells) override;
   void updateMapLayout(const bool singleMapLayout);
   void refreshCurrentPercentageRange();
   void hideAllTooltips() override;
   void finalizeTooltip(const std::vector<double>& lithofractionsAtPoint, const QString& wellName, const int plotID);

private:
   virtual const MapPlotOptionsLithofraction* plotOptions() const override;

   std::vector<LithoGrid2DPlot*> lithoFractionPlots_;
   MapPlotOptionsLithofraction* plotOptions_;
   QLabel* lithoSelectionLabel_;
   QGridLayout* plotsAndOptions_;

   void setTotalLayout();
   void initializeLithoFractionPlots();
   void connectSignalsAndSlots();

   std::pair<double, double> getGlobalRange();
   void setThreePlotLayout();
   void setOnePlotLayout(const int activePlot);

signals:
   void updateAvailableLayers();

public slots:
   void setColorMapType(const std::string& colorMapType) override;
   void slotUpdatePercentageRanges(const QString& percentageRangeType);

private slots:
   void slotUpdateAspectRatio(int state);
   void slotUpdateColorMaps(const QString& colormapType);
   void slotUpdateWellsVisibility(int state);
   void slotUpdateActivePlot(int activePlot);
};

} // namespace sac

} // namespace casaWizard
