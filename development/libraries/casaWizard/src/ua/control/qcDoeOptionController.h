// Controller for the table containing the doe options for QC analysis.
#pragma once

#include <QObject>

class QTableWidgetItem;

namespace casaWizard
{

namespace ua
{

class QCDoeOptionTable;
class UAScenario;

class QCDoeOptionController : public QObject
{
  Q_OBJECT

public:
  explicit QCDoeOptionController(QCDoeOptionTable* table, UAScenario& casaScenario, QObject* parent);

signals:
  void modelChange();

private slots:
  void slotTableWidgetQcDoeOptionItemChanged(QTableWidgetItem* item);
  void slotComboBoxProxyOrderItemChanged(QString proxyOrder);
  void slotComboBoxKrigingItemChanged(QString krigingMethod);

  void slotRefresh();

private:
  void updateAfterModification();

  QCDoeOptionTable* table_;
  UAScenario& casaScenario_;
};

} // namespace ua

} // namespace casaWizard
