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
class QDialogButtonBox;

namespace casaWizard
{

class ImportWellPropertyTable;

class ImportWellPopup : public QDialog
{
  Q_OBJECT
public:
  explicit ImportWellPopup(QWidget *parent = nullptr);
  const QTableWidget* propertyMappingTable() const;

  void updateTable(const QStringList& propertyUserNames, const QStringList& defaultCauldronNames, const QStringList& availableCauldronNames);

  QMap<QString, QString> getCurrentMapping() const;
signals:
  void acceptedClicked();

protected:
  QTableWidget* propertyMappingTable_;
  QDialogButtonBox* buttons_;
};

} // namespace casaWizard

