// menu bar for the UA thermal wizard

#include "view/menuBar.h"

namespace casaWizard
{

namespace ua
{

class MenuBarUA : public MenuBar
{
  Q_OBJECT

public:
  explicit MenuBarUA(QWidget* parent = 0);

  QAction* actionRemoveDoeData() const;
  QAction* actionRemoveDoeDataAll() const;

private:
  QAction* actionRemoveDoeData_;
  QAction* actionRemoveDoeDataAll_;
};

} // namespace ua

} // namespace casaWizard
