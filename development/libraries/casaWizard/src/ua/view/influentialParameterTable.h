// Widget to display the list of influential parameters
#pragma once

#include "model/influentialParameter.h"

#include <QWidget>

class QTableWidget;
class QPushButton;

namespace casaWizard
{

namespace ua
{

class InfluentialParameterTable : public QWidget
{
  Q_OBJECT

public:
  explicit InfluentialParameterTable(QWidget* parent = 0);

  void updateTable(const QVector<InfluentialParameter*> influentialParameters, const QStringList& defaultParameters);
  const QTableWidget* tableWidgetInfluentialParameter() const;
  const QPushButton* pushButtonAddInfluentialParameter() const;
  const QPushButton* pushButtonDelInfluentialParameter() const;

signals:
  void popupArguments(const int row);
  void changeInfluentialParameter(const int row, const int selected);

private:
  QTableWidget* tableWidgetInfluentialParameter_;
  QPushButton* pushButtonAddInfluentialParameter_;
  QPushButton* pushButtonDelInfluentialParameter_;
};

} // namespace ua

} // namespace casaWizard
