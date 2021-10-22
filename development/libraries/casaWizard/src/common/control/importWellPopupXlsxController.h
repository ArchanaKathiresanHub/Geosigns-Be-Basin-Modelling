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
  ImportWellPopupXlsxController(QObject* parent, CasaScenario& casaScenario);
  ~ImportWellPopupXlsxController() final;

  int executeImportWellPopup();
  ImportWellPopup* importWellPopup() const final;
  bool importWellsToCalibrationTargetManager(const QString &fileName) final;

private slots:
  void slotAcceptedClicked();

private:
  ImportWellPopupXlsx* importWellPopup_;
};

} // namespace casaWizard
