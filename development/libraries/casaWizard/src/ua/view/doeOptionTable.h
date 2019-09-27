// Widget for a table to select multiple design of experiemnts

#pragma once

#include "model/doeOption.h"
#include <QTableWidget>

namespace casaWizard
{

namespace ua
{

class DoeOptionTable : public QTableWidget
{
  Q_OBJECT

public:
  explicit DoeOptionTable(QWidget* parent = 0);

  void updateTable(const QVector<DoeOption*>& doeOptions, const QVector<bool>& isDoeOptionsSelected);

  int columnIndexNDesignPointsDoeOptionTable() const;
  int columnCheckBoxDoeOptionTable() const;
};

} // namespace ua

} // namespace casaWizard
