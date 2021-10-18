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

class UserPropertyChoicePopup : public QDialog
{
  Q_OBJECT
public:
  explicit UserPropertyChoicePopup(QWidget *parent = nullptr);
  virtual void updateTable(const QStringList& propertyUserNames);
  QStringList selectedProperties() const;

  QTableWidget* propertyTable() const;

signals:
  void acceptedClicked();

protected:
  QString getPropertyIfChecked(int row) const;

private:
  QTableWidget* propertyTable_;
};

} // namespace casaWizard

