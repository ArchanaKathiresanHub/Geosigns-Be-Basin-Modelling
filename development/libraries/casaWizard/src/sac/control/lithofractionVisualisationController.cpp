//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "lithofractionVisualisationController.h"

#include "../common/model/input/mapreader.h"
#include "../common/model/lithofractionmap.h"
#include "model/casaScenario.h"
#include "model/sacScenario.h"

#include "view/plot/grid2Dview.h"
#include "view/grid2dplot.h"
#include "view/lithofractionVisualisation.h"

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
  scenario_{casaScenario},
  lithofractionVisualisation_{lithofractionVisualisation}
{
  connect(lithofractionVisualisation_->layerSelection(), SIGNAL(currentTextChanged(const QString&)), this, SLOT(slotUpdatePlots(const QString&)));
}

void LithofractionVisualisationController::slotUpdatePlots(const QString& layerName)
{
  MapReader mapReader;
  const int layerID = scenario_.projectReader().getLayerID(layerName.toStdString());

  if (!openMaps(mapReader, layerID))
  {
    lithofractionVisualisation_->clearPlots();
    return;
  }

  double xMin, xMax, yMin, yMax;
  scenario_.projectReader().domainRange(xMin, xMax, yMin, yMax);
  const std::vector<LithofractionMap> lithologyMaps = obtainLithologyMaps(mapReader, layerID);
  const QStringList lithologyTypes = obtainLithologyTypes(layerID);

  int counter = 0;
  for (Grid2DPlot* lithoPlot : lithofractionVisualisation_->lithoFractionPlots())
  {
    lithoPlot->setVisible(lithologyTypes[counter] != "");

    lithoPlot->grid2DView()->updatePlots(convertToQData(lithologyMaps[counter].getData()));
    lithoPlot->grid2DView()->updateRange(xMin * Utilities::Maths::MeterToKilometer,
                                         xMax * Utilities::Maths::MeterToKilometer,
                                         yMin * Utilities::Maths::MeterToKilometer,
                                         yMax * Utilities::Maths::MeterToKilometer);
    lithoPlot->updateColorBar();
    lithoPlot->setTitle(lithologyTypes[counter], counter);
    counter ++;
  }
}

bool LithofractionVisualisationController::openMaps(MapReader& mapReader, const int layerID)
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

std::vector<LithofractionMap> LithofractionVisualisationController::obtainLithologyMaps(const MapReader& mapReader, int layerID)
{
  std::vector<LithofractionMap> lithologyMaps;
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

QStringList LithofractionVisualisationController::obtainAvailableLayers()
{
  QDir threeDFromOneD = scenario_.calibrationDirectory() + "/ThreeDFromOneD/";
  if (scenario_.project3dFilename() == "" || !threeDFromOneD.exists())
  {
    return {};
  }

  MapReader mapreader;
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

QStringList LithofractionVisualisationController::obtainLithologyTypes(const int layerID)
{
  QStringList lithologyTypes = scenario_.projectReader().lithologyTypesForLayer(layerID);
  return lithologyTypes;
}

QVector<QVector<double>> LithofractionVisualisationController::convertToQData(const std::vector<std::vector<double>>& mapData)
{
  QVector<QVector<double>> QData;
  for (const std::vector<double>& row : mapData)
  {
    QVector<double> QRow;
    for (const double value : row)
    {
      QRow.push_back(value);
    }
    QData.push_back(QRow);
  }
  return QData;
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
    lithofractionVisualisation_->updateLayerOptions(availableLayers);
    lithofractionVisualisation_->setVisible(true);
    lithofractionVisualisation_->update();
  }
}

} // namespace sac

} // namespace casaWizard
