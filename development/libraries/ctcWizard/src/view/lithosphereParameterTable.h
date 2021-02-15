#pragma once

#include "model/lithosphereParameter.h"

#include <QWidget>

class QTableWidget;
class QComboBox;
class QPushButton;

namespace ctcWizard
{

class LithosphereParameterTable : public QWidget
{
  Q_OBJECT

public:
  explicit LithosphereParameterTable(QWidget *parent = 0);

  void updateTable(const QVector<LithosphereParameter>& varPrms);
  const QTableWidget* tableWidgetLithosphereParameter() const;


private:
  QTableWidget* tableWidgetLithosphereParameter_;
};

} // namespace ctcWizard
