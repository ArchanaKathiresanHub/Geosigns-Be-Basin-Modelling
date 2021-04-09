//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "model/output/SACInfoGenerator.h"

#include <QString>
namespace casaWizard
{

namespace sac
{

class SACScenario;

class T2ZInfoGenerator : public SACInfoGenerator
{
public:
  T2ZInfoGenerator(const SACScenario& scenario, ProjectReader& t2zProjectReader);
  void generateInfoTextFile() override;

private:
  void addT2ZSection();
  void addT2ZRunSettings();
  void addSurfaceInfo();

  void getOptionValue(const QString& surface, const int id);
};

} // namespace sac

} // namespace casaWizard
