// Widget for a table to select multiple design of experiments in SA tab

#pragma once

#include <QWidget>

class QComboBox;
class QTableWidget;

namespace casaWizard
{

namespace ua
{

class Proxy;

class QCDoeOptionTable : public QWidget
{
  Q_OBJECT

public:
  explicit QCDoeOptionTable(QWidget* parent = 0);

  QTableWidget* tableWidgetQcDoeOption() const;
  void updateTable(const QStringList& doeOptionNames, const QVector<bool>& isQcDoeOptionsSelected);
  void setProxyComboBoxes(const Proxy& proxy);

  int columnCheckBoxQcDoeOptionTable() const;

  QComboBox* comboBoxProxyOrder() const;
  QComboBox* comboBoxKriging() const;

private:
  QTableWidget* tableWidgetQcDoeOption_;
  QComboBox* comboBoxProxyOrder_;
  QComboBox* comboBoxKriging_;
};

} // namespace ua

} // namespace casaWizard
