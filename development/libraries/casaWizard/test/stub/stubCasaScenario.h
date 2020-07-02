#pragma once
#include "model/casaScenario.h"
#include "stubProjectReader.h"

#include <memory>
namespace casaWizard
{

class StubCasaScenario : public CasaScenario
{
public:
  explicit StubCasaScenario() :
    CasaScenario(std::unique_ptr<ProjectReader>(new StubProjectReader()))
  {
  }
};

} // casaWizard
