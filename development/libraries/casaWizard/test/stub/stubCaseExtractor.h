//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "model/extractor/caseExtractor.h"
#include "stubSACScenario.h"

#include <map>

namespace casaWizard
{

class RunScript;

namespace sac
{

namespace lithology
{

class StubCaseExtractor : public CaseExtractor
{
public :
  explicit StubCaseExtractor(LithologyScenario& scenario);
  RunScript& script() override { throw std::out_of_range("Does not exist in this stub extractor"); }
  CaseDataCreator& dataCreator() override { throw std::out_of_range("Does not exist in this stub extractor"); }

  const std::map<int, QString>& updatedCases() const {return updatedCases_;}

private:
  void updateCaseScript(const int wellIndex, const QString relativeDataFolder) override;
  std::map<int, QString> updatedCases_;
};

StubCaseExtractor::StubCaseExtractor(LithologyScenario& scenario) :
  CaseExtractor(scenario),
  updatedCases_{}
{

}

void StubCaseExtractor::updateCaseScript(const int wellIndex, const QString relativeDataFolder)
{
  updatedCases_.insert({wellIndex, relativeDataFolder});
}

} // namespace lithology

} // namespace sac

} // namespace casaWizard
