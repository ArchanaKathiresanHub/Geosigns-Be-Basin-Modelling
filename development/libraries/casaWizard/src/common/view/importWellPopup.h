//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QDialog>

class QTableWidget;

namespace casaWizard
{

class ImportWellPropertyTable;

class ImportWellPopup : public QDialog
{
  Q_OBJECT
public:
  explicit ImportWellPopup(QWidget *parent = nullptr);
  void updateTable(const QStringList& propertyUserNames, const QStringList& defaultCauldronNames, const QStringList& availableCauldronNames);
  const ImportWellPropertyTable* propertyMappingTable() const;

signals:
  void acceptedClicked();

private:
  ImportWellPropertyTable* propertyMappingTable_;
};

} // namespace casaWizard

