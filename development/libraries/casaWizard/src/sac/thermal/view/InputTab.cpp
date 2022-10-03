#include "InputTab.h"
#include <QPushButton>

namespace casaWizard
{

namespace sac
{

namespace thermal
{

InputTab::InputTab(QWidget* parent) :
   SacInputTab(parent),
   m_pushImportTargets{new QPushButton("Import Targets", this)}
{
   addImportButton(m_pushImportTargets);
}

} // namespace thermal

} // namespace sac

} // namespace casaWizard
