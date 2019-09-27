// Table for setting the lithofractions as influential parameters
#pragma once

#include <QWidget>

class QTableWidget;
class QPushButton;

namespace casaWizard
{

class ProjectReader;

namespace sac
{

struct Lithofraction;

class LithofractionTable : public QWidget
{
  Q_OBJECT

public:
  explicit LithofractionTable(QWidget* parent = 0);

  void updateTable(const QVector<Lithofraction>& lithofractions, const ProjectReader& projectReader);

  const QTableWidget* table() const;
  const QPushButton* addRow() const;
  const QPushButton* delRow() const;
  const QPushButton* fillLayers() const;

signals:
  void layersFromProject();

private:
  QTableWidget* table_;
  QPushButton* addRow_;
  QPushButton* delRow_;
  QPushButton* fillLayers_;
};

} // namespace sac

} // namespace casaWizard
