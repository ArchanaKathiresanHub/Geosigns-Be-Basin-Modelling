//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "importWellPopupController.h"
#include "model/input/extractWellDataLAS.h"

namespace casaWizard
{

class ImportWellPopupLAS;

class ImportWellPopupLASController : public ImportWellPopupController
{
  Q_OBJECT
public:
  explicit ImportWellPopupLASController(QObject* parent, CasaScenario& casaScenario);
  ~ImportWellPopupLASController() final;
  ImportWellPopup* importWellPopup() const final;
  bool importWellsToCalibrationTargetManager(const QString& fileName) final;

private slots:
  void slotAcceptedClicked();

private:
  int executeImportWellPopup();
  QVector<double> getUnitConversions(const QStringList& units);

  ImportOptions options_;
  ImportWellPopupLAS* importWellPopup_;
};

} // namespace casaWizard

