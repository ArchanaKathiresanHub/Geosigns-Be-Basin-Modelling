#pragma once

#include <QTableWidget>

namespace casaWizard
{

class ObjectiveFunctionManager;

class ObjectiveFunctionTable : public QTableWidget
{
  Q_OBJECT

public:
  explicit ObjectiveFunctionTable(QWidget* parent = 0);

  void updateTable(const ObjectiveFunctionManager& objectiveFunction);  
};

}  // namespace casaWizard
