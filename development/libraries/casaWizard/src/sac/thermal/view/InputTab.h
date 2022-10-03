#include "view/SacInputTab.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

class InputTab : public SacInputTab
{
public:
   explicit InputTab(QWidget* parent = nullptr);

   const QPushButton* pushButtonImportTargets() const { return m_pushImportTargets; }
private:
   QPushButton* m_pushImportTargets;
};

} //namespace thermal

} //namespace sac

} //namespace casaWizard
