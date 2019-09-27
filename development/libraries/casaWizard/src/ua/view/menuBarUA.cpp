#include "menuBarUA.h"

namespace casaWizard
{

namespace ua
{

MenuBarUA::MenuBarUA(QWidget* parent) :
  MenuBar(parent),
  actionRemoveDoeData_{new QAction("Remove DoE data (current run only)", this)},
  actionRemoveDoeDataAll_{new QAction("Remove DoE data (all)", this)}
{
  menuOptions()->addAction(actionRemoveDoeData_);
  menuOptions()->addAction(actionRemoveDoeDataAll_);
}

QAction* MenuBarUA::actionRemoveDoeData() const
{
  return actionRemoveDoeData_;
}

QAction* MenuBarUA::actionRemoveDoeDataAll() const
{
  return actionRemoveDoeDataAll_;
}


} // namespace ua

} // namespace casaWizard
