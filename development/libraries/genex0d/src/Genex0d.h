//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Genex0d class: is the main API class for genex0d

#pragma once

#include "Genex0dProjectManager.h"

#include <memory>
#include <string>

namespace Genex0d
{

class Genex0dFormationManager;
struct Genex0dInputData;
class Genex0dProjectManager;
class Genex0dSimulator;
class Genex0dSimulatorFactory;

class Genex0d
{
public:
  Genex0d(const Genex0dInputData & inputData);
  ~Genex0d();

  void initialize();
  void run();
  void printResults(const std::string & outputFileName) const;

private:
  void loadSimulator();
  void loadFormation();
  void loadProjectMgr();

  const Genex0dInputData & m_inData;
  std::unique_ptr<Genex0dFormationManager> m_formationMgr;
  std::unique_ptr<Genex0dProjectManager> m_projectMgr;
  std::unique_ptr<Genex0dSimulatorFactory> m_gnx0dSimulatorFactory;
  std::unique_ptr<Genex0dSimulator> m_gnx0dSimulator;
};

} // namespace genex0d
