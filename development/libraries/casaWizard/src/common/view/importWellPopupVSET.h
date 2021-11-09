//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "importWellPopup.h"

#include "model/input/importOptions.h"

class QComboBox;
class QLabel;
class QLineEdit;
class QStackedLayout;

namespace casaWizard
{  

class ImportWellPopupVSET : public ImportWellPopup
{
  Q_OBJECT
public:
  explicit ImportWellPopupVSET(QWidget* parent = nullptr);

  void updateTableUsingOptions(const ImportOptionsVSET& importOptions);

  ImportOptionsVSET getImportOptions();

private slots:
  void slotHandleFilterOptionChanged(int option);
  void slotHandleSetInterval(int interval);
  void slotHandleSetDistance(int distance);
  void slotSelectWellsFile();

private:
  ImportOptionsVSET importOptions_;

  QComboBox* filterOptions_;
  QStackedLayout* filterLayout_;

  QLabel* selectedFileLabel_;

  QLineEdit* createWellIdentifierWidget(QLabel* wellNameLabel);
  QWidget* createSkipWidget() const;
  QWidget* createDistanceWidget() const;
  QWidget* createWellsFileWidget();
};

} // namespace casaWizard
