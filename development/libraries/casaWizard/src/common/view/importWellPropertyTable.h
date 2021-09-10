//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QTableWidget>

class QStringList;

namespace casaWizard
{

class ImportWellPropertyTable : public QTableWidget
{
  Q_OBJECT
public:
  explicit ImportWellPropertyTable(QWidget *parent = nullptr);
  void updateTable(const QStringList& propertyUserNames, const QStringList& defaultCauldronNames, const QStringList& availableCauldronNames);
  QMap<QString, QString> getCurrentMapping() const;
};

} // namespace casaWizard
