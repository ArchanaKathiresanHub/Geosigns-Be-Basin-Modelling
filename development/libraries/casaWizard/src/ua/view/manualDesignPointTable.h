// Widget for adding manual design points
#pragma once

#include <QWidget>

class QPushButton;
class QTableWidget;

namespace casaWizard
{

namespace ua
{

class ManualDesignPointTable : public QWidget
{
  Q_OBJECT

public:
  explicit ManualDesignPointTable(QWidget* parent = 0);

  const QPushButton* buttonAdd() const;
  const QPushButton* buttonDelete() const;
  const QTableWidget* table() const;

  void updateTable(const QVector<QVector<double>> parameters, const QVector<bool> completed, const QStringList headers);

private:
  void setColumnHeaders(const QStringList headers);

  QTableWidget* tableDesignPoints_;
  QPushButton* buttonAdd_;
  QPushButton* buttonDelete_;
};

} // namespace ua

} // namespace casaWizard
