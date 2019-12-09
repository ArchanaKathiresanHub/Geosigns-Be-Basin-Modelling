// Data class for the SAC Wizard
#pragma once

#include "model/casaScenario.h"
#include "model/lithofractionManager.h"
#include "model/wellTrajectoryManager.h"

namespace casaWizard
{

namespace sac
{

class SACScenario : public CasaScenario
{
public:
  SACScenario(std::unique_ptr<ProjectReader> projectReader);

  QString stateFileNameSAC() const;
  void setStateFileNameSAC(const QString& stateFileNameSAC);
  QString calibrationDirectory() const;

  int referenceSurface() const;
  void setReferenceSurface(int referenceSurface);

  int lastSurface() const;
  void setLastSurface(int lastSurface);

  LithofractionManager& lithofractionManager();
  const LithofractionManager& lithofractionManager() const;

  WellTrajectoryManager& wellTrajectoryManager();
  const WellTrajectoryManager& wellTrajectoryManager() const;

  void writeToFile(ScenarioWriter& writer) const override;
  void readFromFile(const ScenarioReader& reader) override;
  void clear() override;
  QString iterationDirName() const override;


  QVector<bool> activePlots() const;
  void setActivePlots(const QVector<bool>& activePlots);

private:
  QString stateFileNameSAC_;
  QString calibrationFolder_;
  LithofractionManager lithofractionManager_;
  WellTrajectoryManager wellTrajectoryManager_;

  int referenceSurface_;
  int lastSurface_;
  QVector<bool> activePlots_;
};

} // namespace sac

} // namespace casaWizard
