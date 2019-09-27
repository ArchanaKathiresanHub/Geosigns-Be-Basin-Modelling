// UA Thermal wizard tab with the quality check results
#pragma once

#include <QVector>
#include <QWidget>

class QCheckBox;
class QPushButton;
class QTableWidget;

namespace casaWizard
{

namespace ua
{

class QCPlot;
class QCDoeOptionTable;
struct TargetQC;

class QCTab : public QWidget
{
  Q_OBJECT

public:
  explicit QCTab(QWidget* parent = 0);
  const QPushButton* pushButtonQCrunCASA() const;
  QPushButton* pushButtonQCrunCASA();
  QTableWidget* tableQC() const;
  void fillQCtable(const QVector<TargetQC>& targetQCs);
  void updateQCPlot(const TargetQC& targetQC);

  QCDoeOptionTable* qcDoeOptionTable() const;

private:
  QPushButton* pushButtonSArunCASA_;
  QTableWidget* tableQC_;
  QCPlot* qcPlot_;
  QCDoeOptionTable* qcDoeOptionTable_;
};

} // namespace ua

} // namespace casaWizard
