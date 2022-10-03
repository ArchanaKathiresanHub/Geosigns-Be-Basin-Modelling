#pragma once

#include <QWidget>
#include "view/SacInputTab.h"

namespace casaWizard
{

namespace sac
{

class LithofractionTable;

namespace lithology {

class InputTab : public SacInputTab
{
   Q_OBJECT
public:
   explicit InputTab(QWidget* parent = 0);
   LithofractionTable* lithofractionTable() const;
private:
   LithofractionTable* lithofractionTable_;
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard
