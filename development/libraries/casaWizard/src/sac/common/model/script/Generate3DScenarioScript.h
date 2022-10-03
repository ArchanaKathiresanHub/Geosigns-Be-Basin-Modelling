// Class for creating the optimized 3D case
#pragma once

#include "model/script/casaScript.h"

namespace casaWizard
{

namespace sac
{
class MapsManager;
}

namespace sac
{

class SacScenario;

class Generate3DScenarioScript : public CasaScript
{
public:
   explicit Generate3DScenarioScript(const SacScenario& scenario);

   const CasaScenario& scenario() const override;
   QString scriptFilename() const override;
   QString workingDirectory() const override;

private:
   void writeScriptContents(QFile& file) const override;
   bool validateScenario() const  override;
   QString setFilterOneDResults() const;
   QString generateThreeDFromOneD() const;

   const MapsManager& mapsManager_;
   const SacScenario& scenario_;
};

} // namespace sac

} // namespace casaWizard
