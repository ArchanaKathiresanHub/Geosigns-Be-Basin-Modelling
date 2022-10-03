#pragma once

#include "control/SacInputController.h"
#include <memory>

namespace casaWizard
{

namespace sac
{

class SACScript;

namespace thermal
{

class InputTab;

class InputControllerThermal : public SacInputController
{
   Q_OBJECT

public:
   InputControllerThermal(InputTab* inputTab,
                          SacScenario& casaScenario,
                          ScriptRunController& scriptRunController,
                          QObject* parent);

private slots:
   void slotImportTargetsClicked();

private:

   void slotUpdateTabGUI(int tabID);
   std::unique_ptr<SACScript> optimizationScript(const QString& baseDirectory, bool doOptimization = true);
   SacScenario& scenario();
   SacScenario& scenario() const;

   void readOptimizedResults();
   InputTab* m_inputTab;
   SacScenario& m_scenario;
};

} //namespace thermal

} //namespace sac

} //namespace casaWizard


