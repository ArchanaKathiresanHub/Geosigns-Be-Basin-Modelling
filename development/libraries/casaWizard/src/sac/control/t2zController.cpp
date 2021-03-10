#include "t2zController.h"

#include "control/casaScriptWriter.h"
#include "control/scriptRunController.h"
#include "model/logger.h"
#include "model/sacScenario.h"
#include "model/scenarioBackup.h"
#include "model/script/depthConversionScript.h"
#include "model/script/sacScript.h"
#include "model/input/calibrationTargetCreator.h"
#include "view/t2zTab.h"

#include <QDir>
#include <QSpinBox>
#include <QPushButton>
#include <QString>

namespace casaWizard
{

namespace sac
{

T2Zcontroller::T2Zcontroller(T2Ztab* t2zTab,
                             SACScenario& casaScenario,
                             ScriptRunController& scriptRunController,
                             QObject* parent) :
    QObject(parent),
    t2zTab_{t2zTab},
    casaScenario_{casaScenario},
    scriptRunController_{scriptRunController}
{
  t2zTab_->spinBoxReferenceSurface()->setValue(casaScenario.referenceSurface());
  t2zTab_->spinBoxLastSurface()->setValue(casaScenario.lastSurface());

  connect(t2zTab_->pushButtonSACrunT2Z(),     SIGNAL(clicked()),
          this,                               SLOT(slotPushButtonSACrunT2ZClicked()));
  connect(t2zTab_->spinBoxReferenceSurface(), SIGNAL(valueChanged(int)),
          this,                               SLOT(slotSpinBoxReferenceSurfaceValueChanged(int)));
  connect(t2zTab_->spinBoxLastSurface(),      SIGNAL(valueChanged(int)),
          this,                               SLOT(slotSpinBoxLastSurfaceValueChanged(int)));
}

void T2Zcontroller::slotPushButtonSACrunT2ZClicked()
{
  scenarioBackup::backup(casaScenario_);
  const QString workingDir = casaScenario_.workingDirectory();

  const QString calibrationDir{casaScenario_.calibrationDirectory()};
  const QString t2zDir{workingDir + "/T2Z_step2"};
  const QString recalibrationDir{workingDir + "/recalibration_step3"};
  const QString calibratedProjectDir{workingDir + "/CalibratedProject"};

  QDir(t2zDir).removeRecursively();
  QDir(recalibrationDir).removeRecursively();
  QDir(calibratedProjectDir).removeRecursively();

  QDir().mkdir(t2zDir);
  const QString projectFilename{QDir::separator() + QFileInfo(casaScenario_.project3dPath()).fileName()};
  QFile::copy(calibrationDir + "/ThreeDFromOneD" + projectFilename, t2zDir + projectFilename);
  QFile::copy(calibrationDir + "/ThreeDFromOneD/Inputs.HDF", t2zDir + "/Inputs.HDF");
  if (QFile(calibrationDir + "/ThreeDFromOneD/Input.HDF").exists())
  {
    QFile::copy(calibrationDir + "/ThreeDFromOneD/Input.HDF", t2zDir + "/Input.HDF");
  }

  DepthConversionScript depthConversion{casaScenario_, t2zDir};
  if (scriptRunController_.runScript(depthConversion))
  {
    QDir().mkdir(recalibrationDir);
    QFile::copy(t2zDir + "/CalibratedDepthMapsProject" + projectFilename, recalibrationDir + projectFilename);
    QFile::copy(t2zDir + "/CalibratedDepthMapsProject/Inputs.HDF", recalibrationDir + "/Inputs.HDF");
    if (QFile(t2zDir + "/CalibratedDepthMapsProject/Input.HDF").exists())
    {
      QFile::copy(t2zDir + "/CalibratedDepthMapsProject/Input.HDF", recalibrationDir + "/Input.HDF");
    }

    SACScript sac{casaScenario_, recalibrationDir};
    if (!casaScriptWriter::writeCasaScript(sac) ||
        !scriptRunController_.runScript(sac))
    {
      return;
    }

    QDir().mkdir(calibratedProjectDir);
    QFile::copy(recalibrationDir + "/ThreeDFromOneD" + projectFilename, calibratedProjectDir + projectFilename);
    QFile::copy(recalibrationDir + "/ThreeDFromOneD/Inputs.HDF", calibratedProjectDir + "/Inputs.HDF");
    if (QFile(recalibrationDir + "/ThreeDFromOneD/Input.HDF").exists())
    {
      QFile::copy(recalibrationDir + "/ThreeDFromOneD/Input.HDF", calibratedProjectDir + "/Input.HDF");
    }
    scenarioBackup::backup(casaScenario_);
  }
}

void T2Zcontroller::slotSpinBoxReferenceSurfaceValueChanged(int referenceSurface)
{
  casaScenario_.setReferenceSurface(referenceSurface);
}

void T2Zcontroller::slotSpinBoxLastSurfaceValueChanged(int lastSurface)
{
  casaScenario_.setLastSurface(lastSurface);
}


} // namespace sac

} // namespace casaWizard
