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

#include "view/components/customcheckbox.h"
#include "view/grid2dplot.h"
#include "view/lithofractionVisualisation.h"
#include "view/plot/lithoPercent2Dview.h"

#include "ConstantsMathematics.h"
#include "ConstantsNumerical.h"

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
  singleMapLayout_{false},
  lithofractionVisualisation_{lithofractionVisualisation},
  scenario_{casaScenario}
{
  connect(parent, SIGNAL(signalRefreshChildWidgets()), this, SLOT(slotRefresh()));
  connect(lithofractionVisualisation_->singleMapLayout(), SIGNAL(stateChanged(int)), this, SLOT(slotUpdateSingleMapLayout(int)));

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
  if (layerName == "")
  {
    return;
  }

  activeLayer_ = layerName;
  const int layerID = scenario_.projectReader().getLayerID(layerName.toStdString());

  CMBMapReader mapReader;
  const bool optimizedLithomapsAvailable = openMaps(mapReader, layerID);

  double xMin = 0; double xMax = 1; double yMin = 0; double yMax = 1;
  scenario_.projectReader().domainRange(xMin, xMax, yMin, yMax);
  std::vector<VectorVectorMap> lithologyMaps;

  if (optimizedLithomapsAvailable)
  {
    lithologyMaps = obtainLithologyMaps(mapReader, layerID);
  }
  else
  {
    mapReader.load(scenario_.project3dPath().toStdString());
  }

  VectorVectorMap depthMap = mapReader.getMapData(scenario_.projectReader().getDepthGridName(0).toStdString());
  const QStringList lithologyTypes = obtainLithologyTypes(layerID);

  lithofractionVisualisation_->lithotypeSelection()->clear();
  lithofractionVisualisation_->lithotypeSelection()->addItems(lithologyTypes);

  int counter = 0;
  for (Grid2DPlot* lithoPlot : lithofractionVisualisation_->lithoFractionPlots())
  {
    lithoPlot->setVisible(lithologyTypes[counter] != "" && !(counter>0 && singleMapLayout_));

    std::vector<std::vector<double>> values;
    if (optimizedLithomapsAvailable)
    {
      lithoPlot->lithoPercent2DView()->setSingleValue(false);
      values = lithologyMaps[counter].getData();
      lithoPlot->showColorBar();
      lithofractionVisualisation_->refreshCurrentPercentageRange();
    }
    else
    {
      values = depthMap.getData();
      lithoPlot->lithoPercent2DView()->setSingleValue(true);
      lithoPlot->lithoPercent2DView()->setFixedValueRange({0,1});
      lithoPlot->hideColorBar();
    }

    lithoPlot->lithoPercent2DView()->updatePlots(values, depthMap.getData());
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

std::vector<double> LithofractionVisualisationController::getLithopercentagesAtLocation(const QPointF& point) const
{
  std::vector<double> lithofractionsAtPoint;
  for (const Grid2DPlot* lithoFractionPlot : lithofractionVisualisation_->lithoFractionPlots())
  {
    lithofractionsAtPoint.push_back(lithoFractionPlot->lithoPercent2DView()->getValue(point));
  }

  return lithofractionsAtPoint;
 }

void LithofractionVisualisationController::toolTipCreated(const QPointF& point, const int plotID)
{
  lithofractionVisualisation_->hideAllTooltips();

  std::vector<double> lithofractionsAtPoint;
  int closestWellID = -1;
  if (lithofractionVisualisation_->wellsVisible()->checkState() == Qt::CheckState::Checked)
  {
    lithofractionsAtPoint = getLithopercentagesOfClosestWell(point, closestWellID);
  }
  else if (!lithofractionVisualisation_->lithoFractionPlots()[0]->lithoPercent2DView()->singleValue())
  {
    lithofractionsAtPoint = getLithopercentagesAtLocation(point);
  }

  if (!lithofractionsAtPoint.empty() && lithofractionIsValid(lithofractionsAtPoint[0]))
  {
    QString wellName = "";
    if (closestWellID != -1)
    {
      const Well& closestWell = scenario_.calibrationTargetManager().well(closestWellID);
      wellName = closestWell.name();
      lithofractionVisualisation_->lithoFractionPlots()[plotID]->lithoPercent2DView()->moveTooltipToDomainLocation(QPointF(closestWell.x() * Utilities::Maths::MeterToKilometer,
                                                                                                                           closestWell.y() * Utilities::Maths::MeterToKilometer));
    }

    lithofractionVisualisation_->finalizeTooltip(lithofractionsAtPoint, wellName, plotID);
  }
}

std::vector<double> LithofractionVisualisationController::getLithopercentagesOfClosestWell(const QPointF& point, int& closestWellID) const
{
  std::vector<double> lithofractionsAtPoint;

  double xMin = 0; double xMax = 1; double yMin = 0; double yMax = 1;
  scenario_.projectReader().domainRange(xMin, xMax, yMin, yMax);
  double smallestDistance2 = ((xMax - xMin) / 30)*((xMax - xMin) / 30);

  int closestLithofractionIndex = -1;
  int currentLithofractionIndex = 0;

  const CalibrationTargetManager& calibrationTargetManager = scenario_.calibrationTargetManager();
  const QVector<OptimizedLithofraction>& lithofractionsInLayer = getOptimizedLithoFractionsInLayer(activeLayer_);
  for (const auto& optimizedLithofraction : lithofractionsInLayer)
  {
    const int wellId = optimizedLithofraction.wellId();
    const auto& currentWell = calibrationTargetManager.well(wellId);
    const double distance2 = (currentWell.x()-point.x() * Utilities::Maths::KilometerToMeter) * (currentWell.x()-point.x() * Utilities::Maths::KilometerToMeter) +
                             (currentWell.y()-point.y() * Utilities::Maths::KilometerToMeter) * (currentWell.y()-point.y() * Utilities::Maths::KilometerToMeter);
    const bool isCloser = distance2 < smallestDistance2;
    if (isCloser)
    {
      smallestDistance2 = distance2;
      closestLithofractionIndex = currentLithofractionIndex;
      closestWellID = wellId;
    }
    currentLithofractionIndex++;
  }

  if (closestLithofractionIndex!= -1)
  {
    lithofractionsAtPoint.push_back(lithofractionsInLayer[closestLithofractionIndex].optimizedPercentageFirstComponent());
    lithofractionsAtPoint.push_back(lithofractionsInLayer[closestLithofractionIndex].optimizedPercentageSecondComponent());
    lithofractionsAtPoint.push_back(lithofractionsInLayer[closestLithofractionIndex].optimizedPercentageThirdComponent());
  }

  return lithofractionsAtPoint;
}

bool LithofractionVisualisationController::lithofractionIsValid(const double lithofractionAtPoint) const
{
  return std::fabs(lithofractionAtPoint - Utilities::Numerical::CauldronNoDataValue) > 1e-5;
}

void LithofractionVisualisationController::slotUpdateSingleMapLayout(int state)
{
  singleMapLayout_ = state == Qt::CheckState::Checked;
  lithofractionVisualisation_->updateMapLayout(singleMapLayout_);
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
  CMBMapReader mapreader;
  mapreader.load((scenario_.calibrationDirectory() + "/ThreeDFromOneD/" + scenario_.project3dFilename()).toStdString());

  QStringList availableLayers;
  QStringList layers = scenario_.projectReader().layerNames();

  for (const QString& layer: layers)
  {
    int layerID = scenario_.projectReader().getLayerID(layer.toStdString());
    if ( !getOptimizedLithoFractionsInLayer(layer).empty() || mapreader.mapExists(std::to_string(layerID) + "_percent_1"))
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
  const QVector<OptimizedLithofraction> optimizedLithoFractions = getOptimizedLithoFractionsInLayer(activeLayer_);
  const CalibrationTargetManager& ctManager = scenario_.calibrationTargetManager();
  lithofractionVisualisation_->updateBirdsView(ctManager.activeAndIncludedWells(), optimizedLithoFractions);
}

void LithofractionVisualisationController::slotRefresh()
{
  updateAvailableLayers();
  updateBirdsView();
}

QVector<OptimizedLithofraction> LithofractionVisualisationController::getOptimizedLithoFractionsInLayer(const QString& layer) const
{
  const LithofractionManager& lithoManager = scenario_.lithofractionManager();
  const QVector<Lithofraction>& lithofractions = lithoManager.lithofractions();
  const CalibrationTargetManager& ctManager = scenario_.calibrationTargetManager();

  QVector<OptimizedLithofraction> optimizedLithoFractions;
  for (const Well* well : ctManager.activeAndIncludedWells())
  {
    QVector<OptimizedLithofraction> tmp = lithoManager.optimizedInWell(well->id());
    for (OptimizedLithofraction lithofraction : tmp)
    {
      if (lithofractions[lithofraction.lithofractionId()].layerName() == layer)
      {
        optimizedLithoFractions.push_back(lithofraction);
      }
    }
  }

  return optimizedLithoFractions;
}

} // namespace sac

} // namespace casaWizard
