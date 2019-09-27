//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Genex0d class: is the main API class for genex0d

#pragma once

#include <memory>
#include <string>

namespace genex0d
{

class Genex0dFormationManager;
class Genex0dInputData;
class Genex0dProjectManager;
class Genex0dSourceRock;

class Genex0d
{
public:
  Genex0d(const Genex0dInputData & inputData);
  ~Genex0d();

  void run();
  void printResults(const std::string & outputFileName) const;

private:
  void initialize();
  void setSourceRockInput(const double inorganicDensity);

  const Genex0dInputData & m_inData;
  std::unique_ptr<Genex0dFormationManager> m_formationMgr;
  std::unique_ptr<Genex0dProjectManager> m_projectMgr;
  std::unique_ptr<Genex0dSourceRock> m_sourceRock;
};

} // namespace genex0d
