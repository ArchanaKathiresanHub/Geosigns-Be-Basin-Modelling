//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "lithofractionVisualisationController.h"


#include "../common/model/input/cmbMapReader.h"
#include "../common/model/VectorVectorMap.h"
#include "model/casaScenario.h"
#include "model/sacScenario.h"
#include "model/well.h"

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

  const QString curLithotype = currentlyDisplayedLithotype();
  const int curIndex = lithofractionVisualisation_->lithotypeSelection()->currentIndex();

  activeLayer_ = layerName;

  double xMin = 0; double xMax = 1; double yMin = 0; double yMax = 1;
  scenario_.projectReader().domainRange(xMin, xMax, yMin, yMax);

  std::vector<VectorVectorMap> lithologyMaps;
  QStringList lithologyTypes;
  bool optimizedLithomapsAvailable = scenario_.getLithologyTypesAndMaps(activeLayer_, lithologyMaps, lithologyTypes);

  lithofractionVisualisation_->lithotypeSelection()->clear();
  lithofractionVisualisation_->lithotypeSelection()->addItems(lithologyTypes);

  int counter = 0;
  for (Grid2DPlot* lithoPlot : lithofractionVisualisation_->lithoFractionPlots())
  {
    lithoPlot->setVisible(lithologyTypes[counter] != "" && !(counter>0 && singleMapLayout_));

    std::vector<std::vector<double>> values;

    values = lithologyMaps[counter].getData();
    lithoPlot->showColorBar();
    lithofractionVisualisation_->refreshCurrentPercentageRange();

    lithoPlot->lithoPercent2DView()->updatePlots(values);
    lithoPlot->lithoPercent2DView()->updateRange(xMin * Utilities::Maths::MeterToKilometer,
                                                 xMax * Utilities::Maths::MeterToKilometer,
                                                 yMin * Utilities::Maths::MeterToKilometer,
                                                 yMax * Utilities::Maths::MeterToKilometer);
    lithoPlot->updateColorBar();
    lithoPlot->setTitle(lithologyTypes[counter], counter, !optimizedLithomapsAvailable);
    lithoPlot->lithoPercent2DView()->setToolTipVisible(false);
    lithoPlot->lithoPercent2DView()->setToolTipLithotypes(lithologyTypes);

    counter++;
  }

  updateBirdsView();

  //If present, set lithotype to lithotype used in previous plot.
  int idxOfPreviousLithotypeInNewLayer = lithologyTypes.indexOf(curLithotype);
  if (idxOfPreviousLithotypeInNewLayer != -1)
  {
    lithofractionVisualisation_->lithotypeSelection()->setCurrentIndex(idxOfPreviousLithotypeInNewLayer);
  }
  else if (curIndex != -1 && curIndex < lithologyTypes.size() && !lithologyTypes.at(curIndex).isEmpty())
  {
    lithofractionVisualisation_->lithotypeSelection()->setCurrentIndex(curIndex);
  }
}

QString LithofractionVisualisationController::currentlyDisplayedLithotype() const
{
  QString curLithotype = "";

  //Get the name of the currently displayed lithotype:
  const size_t layerID = scenario_.projectReader().getLayerID(activeLayer_.toStdString());
  const QStringList lithologyTypes = scenario_.projectReader().lithologyTypesForLayer(layerID);
  const int idx = lithofractionVisualisation_->lithotypeSelection()->currentIndex();

  if (idx != -1)
  {
    curLithotype = lithologyTypes.at(idx);
  }

  return curLithotype;
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
  emit clearWellListHighlightSelection();

  std::vector<double> lithofractionsAtPoint;
  int closestWellID = -1;
  if (lithofractionVisualisation_->wellsVisible()->checkState() == Qt::CheckState::Checked)
  {
    lithofractionsAtPoint = scenario_.getLithopercentagesOfClosestWell(point.x(), point.y(), activeLayer_, closestWellID);
  }
  else
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
      emit wellClicked(wellName);
    }

    lithofractionVisualisation_->finalizeTooltip(lithofractionsAtPoint, wellName, plotID);
  }
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

void LithofractionVisualisationController::updateSelectedWells(QVector<int> selectedWells)
{
  QVector<int> highlightedWells = scenario_.getHighlightedWells(selectedWells, activeLayer_);
  lithofractionVisualisation_->updateSelectedWells(highlightedWells);
}

void LithofractionVisualisationController::hideAllTooltips()
{
  lithofractionVisualisation_->hideAllTooltips();
}

QStringList LithofractionVisualisationController::obtainAvailableLayers() const
{
  return scenario_.projectReader().layerNames();
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
  lithofractionVisualisation_->hideAllTooltips();
  emit clearWellListHighlightSelection();
  QVector<OptimizedLithofraction> optimizedLithoFractions;
  QVector<const Well*> activeAndIncludedWells;

  scenario_.getVisualisationData(optimizedLithoFractions, activeAndIncludedWells, activeLayer_);

  lithofractionVisualisation_->updateBirdsView(activeAndIncludedWells, optimizedLithoFractions);
}

void LithofractionVisualisationController::slotRefresh()
{
  updateAvailableLayers();
  updateBirdsView();
}

} // namespace sac

} // namespace casaWizard
