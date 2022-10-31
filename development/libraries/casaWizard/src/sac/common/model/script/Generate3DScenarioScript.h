// Class for creating the optimized 3D case
#pragma once

#include "model/script/casaScript.h"

namespace casaWizard
{

namespace sac
{

class SacMapsManager;

class SacScenario;

class Generate3DScenarioScript : public CasaScript
{
public:
   explicit Generate3DScenarioScript(const SacScenario& scenario);
   virtual ~Generate3DScenarioScript() override = default;

   const CasaScenario& scenario() const override;
   QString scriptFilename() const override;
   QString workingDirectory() const override;

protected:
   virtual const SacMapsManager& mapsManager() const = 0;
   virtual QString setFilterOneDResults() const = 0;
   void addWellIndicesToFilter1DResults(QString& command) const;

private:
   void writeScriptContents(QFile& file) const override;
   bool validateScenario() const  override;

   QString generateThreeDFromOneD() const;
   const SacScenario& m_scenario;
};

} // namespace sac

} // namespace casaWizard
