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
  SACScenario(ProjectReader* projectReader);

  QString stateFileNameSAC() const;
  QString calibrationDirectory() const;

  int interpolationMethod() const;
  void setInterpolationMethod(int interpolationMethod);

  int smoothingOption() const;
  void setSmoothingOption(int smoothingOption);

  int pIDW() const;
  void setPIDW(int pIDW);

  int threadsSmoothing() const;
  void setThreadsSmoothing(int threadsSmoothing);

  int radiusSmoothing() const;
  void setRadiusSmoothing(int radiusSmoothing);

  int t2zReferenceSurface() const;
  void setT2zReferenceSurface(int t2zReferenceSurface);

  int t2zLastSurface() const;

  int t2zNumberCPUs() const;
  void setT2zNumberCPUs(int t2zNumberCPUs);

  int t2zSubSampling() const;
  void setT2zSubSampling(int t2zSubSampling);

  bool t2zRunOnOriginalProject() const;
  void setT2zRunOnOriginalProject(bool t2zRunOnOriginalProject);

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
  void updateT2zLastSurface();

private:
  QString stateFileNameSAC_;
  QString calibrationFolder_;
  LithofractionManager lithofractionManager_;
  WellTrajectoryManager wellTrajectoryManager_;

  int interpolationMethod_;
  int smoothingOption_;
  int pIDW_;
  int threadsSmoothing_;
  int radiusSmoothing_; //[m]
  int t2zLastSurface_;
  int t2zReferenceSurface_;
  int t2zSubSampling_;
  bool t2zRunOnOriginalProject_;
  int t2zNumberCPUs_;

  QVector<bool> activePlots_;
};

} // namespace sac

} // namespace casaWizard
