//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "model/output/infoGenerator.h"

namespace casaWizard
{

class ProjectReader;
class MapReader;
class Well;

namespace sac
{

class SACScenario;

class SACInfoGenerator : public casaWizard::InfoGenerator
{
public:
  SACInfoGenerator(const SACScenario& scenario, ProjectReader& projectReader);
  void generateInfoTextFile() override = 0;
  void loadProjectReader(const std::string& projectFileLocation) override;

protected:
  void addGeneralSettingsSection();
  void addInputTabSection();
  void addMapsGenerationSection();
  void addWellsSection();

  const SACScenario& scenario() const;
  const ProjectReader& projectReader() const;

private:
  void addFormationInfo();
  void addInterpolation();
  void addObjectiveFunction();
  void addRunMode();
  void addSmoothing();
  void addWellInfo(const casaWizard::Well* well);

  std::string doubleToFormattedString(const double inputDouble);

  const SACScenario& scenario_;
  ProjectReader& projectReader_;
};

} // namespace sac

} // namespace casaWizard
