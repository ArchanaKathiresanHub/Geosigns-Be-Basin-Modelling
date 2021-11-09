//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "importWellPopupController.h"

#include "model/input/importOptions.h"

namespace casaWizard
{

class ImportWellPopupVSET;

class ImportWellPopupVSETController : public ImportWellPopupController
{
  Q_OBJECT
public:
  explicit ImportWellPopupVSETController(QObject* parent, CasaScenario& casaScenario);
  ~ImportWellPopupVSETController() final;

  int executeImportWellPopup();
  ImportWellPopup* importWellPopup() const final;
  void importWellsToCalibrationTargetManager(const QString& fileName, CalibrationTargetManager& calibrationTargetManager) final;

private slots:
  void slotAcceptedClicked();

private:
  ImportOptionsVSET options_;
  ImportWellPopupVSET* importWellPopup_;
};

} // namespace casaWizard

