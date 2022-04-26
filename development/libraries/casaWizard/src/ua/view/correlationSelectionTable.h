// Widget for a table to select influential parameters, intended for plotting the correlation matrix
#pragma once

#include <QWidget>

class QTableWidget;

namespace casaWizard
{

namespace ua
{

class InfluentialParameter;

class CorrelationSelectionTable : public QWidget
{
  Q_OBJECT

public:
  explicit CorrelationSelectionTable(QWidget* parent = 0);

  void updateTable(const QVector<InfluentialParameter*> influentialParameters, const QVector<bool>& isInfluentialParameterSelected);
  QVector<bool> getStatus() const;
  QTableWidget* tableParameters() const;

private:
  QTableWidget* tableParameters_;
};

} // namespace ua

} // namespace casaWizard
