//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Project reader using the Cauldron Model Building (CMB) API for reading of the project 3d file
// The mbapi::Model can be constructed without loading. Therefore a status is added to this class, i.e., calling other methods on the mbapi::Model otherwise results in segmentation faults.
#pragma once

#include "projectReader.h"

#include <map>
#include <memory>
#include <QVector>

namespace mbapi
{
  class Model;
}
namespace casaWizard
{

class CMBProjectReader : public ProjectReader
{
public:
  explicit CMBProjectReader();
  ~CMBProjectReader() override;

  void load(const QString& projectFile) override;

  QStringList layerNames() const override;
  QStringList lithologyNames() const override;
  QStringList mapNames() const override;
  QStringList lithologyTypesForLayer(const int layerIndex) const override;

  double heatProductionRate() const override;
  double initialLithosphericMantleThickness() const override;
  double equilibriumOceanicLithosphereThickness() const override;

  QVector<double> agesFromMajorSnapshots() const override;

  void setRelevantOutputParameters(const QStringList& activeProperties, const std::string& saveName = "") override;

  std::map<std::string, std::string> readOutputProperties() const;

private:
  std::unique_ptr<mbapi::Model> cmbModel_;
  bool loaded_;
};

} // namespace casaWizard
