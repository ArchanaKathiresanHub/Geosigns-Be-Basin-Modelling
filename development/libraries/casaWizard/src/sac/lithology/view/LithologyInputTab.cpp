#include "LithologyInputTab.h"

#include "view/lithofractionTable.h"

namespace casaWizard
{

namespace sac
{

namespace lithology {

LithologyInputTab::LithologyInputTab(QWidget* parent) :
   SacInputTab(parent),
   m_lithofractionTable{new LithofractionTable(this)}
{
   addWidget(m_lithofractionTable,"Lithofractions");
}

LithofractionTable* LithologyInputTab::lithofractionTable() const
{
   return m_lithofractionTable;
}

} // namespace lithology

} // namespace sac

} // namespace casaWizard
