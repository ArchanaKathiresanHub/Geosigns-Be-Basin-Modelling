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

class ResponseSurfacesTab : public QWidget
{
  Q_OBJECT

public:
  explicit ResponseSurfacesTab(QWidget* parent = 0);

  const QPushButton* pushButtonQCrunCASA() const;
  QCDoeOptionTable* qcDoeOptionTable() const;
  QTableWidget* tableQC() const;

  void allowModification(bool allow = true);
  void fillQCtable(const QVector<TargetQC>& targetQCs);
  void updateQCPlot(const TargetQC& targetQC, const int& tableIndex = 0);
  void resetQCPlot();

private:
  QPushButton* pushButtonQCrunCASA_;
  QTableWidget* tableQC_;
  QCPlot* qcPlot_;
  QCDoeOptionTable* qcDoeOptionTable_;
};

} // namespace ua

} // namespace casaWizard
