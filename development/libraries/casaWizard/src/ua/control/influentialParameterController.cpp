#include "influentialParameterController.h"

#include "model/influentialParameterManager.h"
#include "view/influentialParameterTable.h"
#include "view/formDialog.h"

#include <QComboBox>
#include <QPushButton>
#include <QString>
#include <QTableWidgetItem>

namespace casaWizard
{

namespace ua
{

InfluentialParameterController::InfluentialParameterController(InfluentialParameterTable* table,
                                                               InfluentialParameterManager& manager,
                                                               QObject* parent) :
  QObject(parent),
  table_(table),
  manager_(manager)
{
  connect(table_->pushButtonAddInfluentialParameter(), SIGNAL(clicked()), this, SLOT(slotPushButtonAddInfluentialParameterClicked()));
  connect(table_->pushButtonDelInfluentialParameter(), SIGNAL(clicked()), this, SLOT(slotPushButtonDelInfluentialParameterClicked()));

  connect(table_, SIGNAL(changeInfluentialParameter(int, int)), this, SLOT(slotChangeInfluentialParameter(int, int)));
  connect(table_, SIGNAL(popupArguments(const int)),            this, SLOT(slotArgumentsPopup(const int)));

  connect(this, SIGNAL(influentialParametersChanged()), SLOT(slotRefresh()));
}

void InfluentialParameterController::slotPushButtonAddInfluentialParameterClicked()
{
  manager_.add();
  emit influentialParametersChanged();
}

void InfluentialParameterController::slotPushButtonDelInfluentialParameterClicked()
{
  int row = table_->tableWidgetInfluentialParameter()->currentRow();
  if (row < 0)
  {
    row = table_->tableWidgetInfluentialParameter()->rowCount()-1;
  }
  if (row >= 0)
  {
    manager_.remove(row);
  }
  emit influentialParametersChanged();
  emit removeInfluentialParameter(row);
}

void InfluentialParameterController::slotChangeInfluentialParameter(const int row, const int selected)
{
  manager_.set(row, selected);
  emit influentialParametersChanged();
}

void InfluentialParameterController::slotArgumentsPopup(const int row)
{
  const InfluentialParameter* influentialParameter = manager_.influentialParameters()[row];

  FormDialog popupForm{influentialParameter->name()};
  const ArgumentList& arguments = influentialParameter->arguments();

  for (int argIndex = 0; argIndex<arguments.size(); ++argIndex)
  {
    switch (arguments.getType(argIndex))
    {
      case ArgumentList::Type::Double:
      {
        popupForm.addOption(arguments.getName(argIndex), arguments.getDoubleArgument(argIndex));
        break;
      }
      case ArgumentList::Type::StringList:
      {
        popupForm.addComboBox(arguments.getName(argIndex), arguments.getListOptions(argIndex), arguments.getSelectedValue(argIndex));
        break;
      }
    }
  }

  if (popupForm.exec() == QDialog::Accepted)
  {
    manager_.setArguments(row, popupForm.optionValues(), popupForm.comboBoxValues());
    emit influentialParametersChanged();
  }
}

void InfluentialParameterController::slotRefresh()
{
  table_->updateTable(manager_.influentialParameters(), manager_.options());
}

} // namespace ua

} // namespace casaWizard
