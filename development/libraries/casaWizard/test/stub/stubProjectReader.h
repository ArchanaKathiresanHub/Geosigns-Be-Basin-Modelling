//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "model/input/projectReader.h"
#include <QVector>

namespace casaWizard
{

class StubProjectReader : public ProjectReader
{
public:
  void load(const QString& /*projectFile*/) override
  {
  }

  QStringList layerNames() const override
  {
    return {"Layer1","Layer2"};
  }

  QStringList surfaceNames() const override
  {
    return {"Water_Bottom", "Second Surface", "Third Surface"};
  }

  QStringList lithologyNames() const override
  {
    return QStringList();
  }

  QStringList mapNames() const override
  {
    return QStringList();
  }

  QStringList lithologyTypesForLayer(const int /*layerIndex*/) const override
  {
    return QStringList();
  }

  QVector<double> lithologyValuesForLayerAtLocation(const int layerIndex, const double xLoc, const double yLoc) const override
  {
    return {};
  }

  size_t getLayerID(const std::string& /*layerName*/) const override
  {
    return 0;
  }

  void domainRange(double& /*xMin*/, double& /*xMax*/, double& /*yMin*/, double& /*yMax*/) const override
  {
  }

  double heatProductionRate() const override
  {
    return 0.0;
  }

  double initialLithosphericMantleThickness() const override
  {
    return 0.0;
  }

  double equilibriumOceanicLithosphereThickness() const override
  {
    return 0.0;
  }

  int lowestSurfaceWithTWTData() const override
  {
    return 10;
  }

  bool basementSurfaceHasTWT() const override
  {
    return true;
  }

  QVector<double> agesFromMajorSnapshots() const override
  {
    return {0, 23, 66, 100, 201, 252, 255, 299};
  }

  bool hasTWTData(int surfaceID) const override
  {
    return surfaceID == 0;
  }

  double getDepth(int surfaceID) const override
  {
     return 0.0;
  }

  QString getDepthGridName(int surfaceID) const override
  {
    return "";
  }

  double getThickness(int surfaceID) const override
  {
    return 0.0;
  }

  bool hasDepthDefinedInAllLayers() const override
  {
    return true;
  }

  QStringList mapNamesT2Z() const override
  {
    return {};
  }

  QString getLayerUnderSurface(const QString &/*surfaceName*/) const override
  {
     return "Layer";
  }
};

} // namespace casaWizard
