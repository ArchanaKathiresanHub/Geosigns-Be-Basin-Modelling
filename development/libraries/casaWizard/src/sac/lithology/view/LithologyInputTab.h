#pragma once

#include <QWidget>
#include "view/SacInputTab.h"

namespace casaWizard
{

namespace sac
{

namespace lithology
{

class LithofractionTable;

class LithologyInputTab : public SacInputTab
{
   Q_OBJECT
public:
   explicit LithologyInputTab(QWidget* parent = 0);
   LithofractionTable* lithofractionTable() const;
private:
   LithofractionTable* m_lithofractionTable;
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard
