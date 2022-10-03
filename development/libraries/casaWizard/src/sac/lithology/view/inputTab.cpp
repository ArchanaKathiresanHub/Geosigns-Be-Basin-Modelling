#include "inputTab.h"

#include "view/lithofractionTable.h"

namespace casaWizard
{

namespace sac
{

namespace lithology {

InputTab::InputTab(QWidget* parent) :
   SacInputTab(parent),
   lithofractionTable_{new LithofractionTable(this)}
{
   addTable(lithofractionTable_,"Lithofractions");
}

LithofractionTable* InputTab::lithofractionTable() const
{
   return lithofractionTable_;
}

} // namespace lithology

} // namespace sac

} // namespace casaWizard
