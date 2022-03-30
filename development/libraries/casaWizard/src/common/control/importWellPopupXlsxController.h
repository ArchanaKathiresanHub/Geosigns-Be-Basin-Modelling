//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "importWellPopupController.h"

namespace casaWizard
{

class ImportWellPopupXlsx;

class ImportWellPopupXlsxController : public ImportWellPopupController
{
  Q_OBJECT
public:
   ImportWellPopupXlsxController(QObject* parent, CasaScenario& casaScenario, const QStringList& allowedProperties = {"TwoWayTime", "GammaRay", "BulkDensity", "SonicSlowness",
                                                                                                                      "Pressure", "Temperature", "VRe", "Velocity", "DT_FROM_VP",
                                                                                                                      "TWT_FROM_DT", "Unknown"});
  ~ImportWellPopupXlsxController() final;

  int executeImportWellPopup(const QStringList& propertyUserNames, const QStringList& defaultCauldronNames);
  ImportWellPopup* importWellPopup() const final;
  void importWellsToCalibrationTargetManager(const QStringList& fileNames, CalibrationTargetManager& calibrationTargetManager) final;

private slots:
  void slotAcceptedClicked();

private:
  ImportWellPopupXlsx* importWellPopup_;
  QStringList allowedProperties_;
};

} // namespace casaWizard
