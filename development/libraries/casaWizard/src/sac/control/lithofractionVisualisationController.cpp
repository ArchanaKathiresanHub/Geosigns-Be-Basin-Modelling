//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "lithofractionVisualisationController.h"


#include "../common/model/input/cmbMapReader.h"
#include "../common/model/vectorvectormap.h"
#include "model/casaScenario.h"
#include "model/sacScenario.h"

#include "view/grid2dplot.h"
#include "view/lithofractionVisualisation.h"
#include "view/plot/lithoPercent2Dview.h"

#include "ConstantsMathematics.h"

#include <QDir>
#include <QComboBox>

namespace casaWizard
{

namespace sac
{

LithofractionVisualisationController::LithofractionVisualisationController(LithofractionVisualisation* lithofractionVisualisation,
                                                                           SACScenario& casaScenario,
                                                                           QObject* parent) :
  QObject{parent},
  lithofractionVisualisation_{lithofractionVisualisation},
  scenario_{casaScenario}
{
  connect(parent, SIGNAL(signalRefreshChildWidgets()), this, SLOT(slotRefresh()));

  connect(lithofractionVisualisation_->layerSelection(), SIGNAL(currentTextChanged(const QString&)), this, SLOT(slotUpdatePlots(const QString&)));
  connectToolTipSlots();
}

void LithofractionVisualisationController::connectToolTipSlots()
{
  int counter = 0;
  for (const Grid2DPlot* lithoFractionPlot : lithofractionVisualisation_->lithoFractionPlots())
  {
    connect(lithoFractionPlot->lithoPercent2DView(), &LithoPercent2DView::toolTipCreated, [=](const QPointF& point){toolTipCreated(point, counter);});
    counter++;
  }
}

void LithofractionVisualisationController::slotUpdatePlots(const QString& layerName)
{
  activeLayer_ = layerName;
  CMBMapReader mapReader;
  const int layerID = scenario_.projectReader().getLayerID(layerName.toStdString());

  if (!openMaps(mapReader, layerID))
  {
    lithofractionVisualisation_->clearPlots();
    return;
  }

  double xMin = 0; double xMax = 1; double yMin = 0; double yMax = 1;
  scenario_.projectReader().domainRange(xMin, xMax, yMin, yMax);
  const std::vector<VectorVectorMap> lithologyMaps = obtainLithologyMaps(mapReader, layerID);

  VectorVectorMap depthMap = mapReader.getMapData(scenario_.projectReader().getDepthGridName(0).toStdString());

  const QStringList lithologyTypes = obtainLithologyTypes(layerID);

  int counter = 0;
  for (Grid2DPlot* lithoPlot : lithofractionVisualisation_->lithoFractionPlots())
  {
    lithoPlot->setVisible(lithologyTypes[counter] != "");    

    lithoPlot->lithoPercent2DView()->updatePlots(lithologyMaps[counter].getData(), depthMap.getData());
    lithoPlot->lithoPercent2DView()->updateRange(xMin * Utilities::Maths::MeterToKilometer,
                                         xMax * Utilities::Maths::MeterToKilometer,
                                         yMin * Utilities::Maths::MeterToKilometer,
                                         yMax * Utilities::Maths::MeterToKilometer);
    lithoPlot->updateColorBar();
    lithoPlot->setTitle(lithologyTypes[counter], counter);
    lithoPlot->lithoPercent2DView()->setToolTipVisible(false);
    lithoPlot->lithoPercent2DView()->setToolTipLithotypes(lithologyTypes);

    counter++;
  }

  updateBirdsView();
}

void LithofractionVisualisationController::toolTipCreated(const QPointF& point, const int plotID)
{
  std::vector<double> lithofractionsAtPoint;

  for (const Grid2DPlot* lithoFractionPlot : lithofractionVisualisation_->lithoFractionPlots())
  {
    lithofractionsAtPoint.push_back(lithoFractionPlot->lithoPercent2DView()->getValue(point));
    lithoFractionPlot->lithoPercent2DView()->setToolTipVisible(false);
  }

  lithofractionVisualisation_->lithoFractionPlots()[plotID]->lithoPercent2DView()->setToolTipVisible(true);
  lithofractionVisualisation_->lithoFractionPlots()[plotID]->lithoPercent2DView()->setToolTipData(lithofractionsAtPoint, plotID);
  lithofractionVisualisation_->lithoFractionPlots()[plotID]->lithoPercent2DView()->correctToolTipPositioning();
}

bool LithofractionVisualisationController::openMaps(CMBMapReader& mapReader, const int layerID) const
{
  QDir threeDFromOneD = scenario_.calibrationDirectory() + "/ThreeDFromOneD/";
  if (scenario_.project3dFilename() == "" || !threeDFromOneD.exists())
  {
    return false;
  }

  mapReader.load((scenario_.calibrationDirectory() + "/ThreeDFromOneD/" + scenario_.project3dFilename()).toStdString());
  if (!mapReader.mapExists(std::to_string(layerID) + "_percent_1"))
  {
    return false;
  }

  return true;
}

std::vector<VectorVectorMap> LithofractionVisualisationController::obtainLithologyMaps(const CMBMapReader& mapReader, int layerID) const
{
  std::vector<VectorVectorMap> lithologyMaps;
  lithologyMaps.push_back(mapReader.getMapData(std::to_string(layerID) + "_percent_1"));

  if (mapReader.mapExists(std::to_string(layerID) + "_percent_2"))
  {
    lithologyMaps.push_back(mapReader.getMapData(std::to_string(layerID) + "_percent_2"));
  }
  else
  {
    lithologyMaps.push_back(lithologyMaps[0] * -1.0 + 100);
  }

  lithologyMaps.push_back((lithologyMaps[0] + lithologyMaps[1]) * (-1.0) + 100);

  return lithologyMaps;
}

QStringList LithofractionVisualisationController::obtainAvailableLayers() const
{
  QDir threeDFromOneD = scenario_.calibrationDirectory() + "/ThreeDFromOneD/";
  if (scenario_.project3dFilename() == "" || !threeDFromOneD.exists())
  {
    return {};
  }

  CMBMapReader mapreader;
  mapreader.load((scenario_.calibrationDirectory() + "/ThreeDFromOneD/" + scenario_.project3dFilename()).toStdString());

  QStringList availableLayers;
  QStringList layers = scenario_.projectReader().layerNames();
  for (const QString& layer: layers)
  {
    int layerID = scenario_.projectReader().getLayerID(layer.toStdString());
    if (mapreader.mapExists(std::to_string(layerID) + "_percent_1"))
    {
      availableLayers.push_back(layer);
    }
  }

  return availableLayers;
}

QStringList LithofractionVisualisationController::obtainLithologyTypes(const int layerID) const
{
  QStringList lithologyTypes = scenario_.projectReader().lithologyTypesForLayer(layerID);
  return lithologyTypes;
}

void LithofractionVisualisationController::updateAvailableLayers()
{
  QStringList availableLayers = obtainAvailableLayers();

  if (availableLayers.empty())
  {
    lithofractionVisualisation_->setVisible(false);
    lithofractionVisualisation_->update();
  }
  else
  {
    activeLayer_ = availableLayers[0];
    lithofractionVisualisation_->updateLayerOptions(availableLayers);
    lithofractionVisualisation_->setVisible(true);    
    lithofractionVisualisation_->update();
  }
}

void LithofractionVisualisationController::updateBirdsView()
{
  const CalibrationTargetManager& ctManager = scenario_.calibrationTargetManager();
  QVector<OptimizedLithofraction> optimizedLithoFractions = getOptimizedLithoFractionsInActiveLayer(ctManager);

  lithofractionVisualisation_->updateBirdsView(ctManager.activeAndIncludedWells(), optimizedLithoFractions);
}

void LithofractionVisualisationController::slotRefresh()
{
  updateAvailableLayers();
  updateBirdsView();
}

QVector<OptimizedLithofraction> LithofractionVisualisationController::getOptimizedLithoFractionsInActiveLayer(const CalibrationTargetManager& ctManager)
{
  const LithofractionManager& lithoManager = scenario_.lithofractionManager();
  const QVector<Lithofraction>& lithofractions = lithoManager.lithofractions();

  QVector<OptimizedLithofraction> optimizedLithoFractions;
  for (const Well* well : ctManager.activeAndIncludedWells())
  {
    QVector<OptimizedLithofraction> tmp = lithoManager.optimizedInWell(well->id());
    for (OptimizedLithofraction lithofraction : tmp)
    {
      if (lithofractions[lithofraction.lithofractionId()].layerName() == activeLayer_)
      {
        optimizedLithoFractions.push_back(lithofraction);
      }
    }
  }

  return optimizedLithoFractions;
}

} // namespace sac

} // namespace casaWizard
