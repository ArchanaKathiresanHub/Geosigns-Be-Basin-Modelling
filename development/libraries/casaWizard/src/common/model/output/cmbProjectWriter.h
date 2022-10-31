//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#pragma once

#include "projectWriter.h"

#include <map>
#include <memory>

#include <QString>

// Project writer using the Cauldron Model Building (CMB) API for reading of the project 3d file
namespace mbapi
{
  class Model;
}

namespace casaWizard
{

class CMBProjectWriter : public ProjectWriter
{
public:
  explicit CMBProjectWriter(const QString& projectFile);
  ~CMBProjectWriter() override;

  void generateOutputProject(const QString& timeStamp, const QString& originalProject) override;
  void setRelevantOutputParameters(const QStringList& activeProperties) override;
  void setScaling(int scaleX, int scaleY) override;

private:
  std::unique_ptr<mbapi::Model> cmbModel_;

  void copyFilterTimeIoTbl(const QString& projectFile);
  void appendTimeStampToCalibratedLithoMaps(const QString& timeStamp);
  void appendTimeStampToT2ZMaps(const QString& timeStamp);
  void appendTimeStampToCalibratedTCHPMap(const QString& timeStamp);
  void deleteOutputTables();
};

} // namespace casaWizard
