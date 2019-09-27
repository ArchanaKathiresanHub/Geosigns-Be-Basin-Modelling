// General menu bar for the wizard

#include <QMenuBar>
#include <QAction>
#include <QMenu>

namespace casaWizard
{

class MenuBar : public QMenuBar
{
  Q_OBJECT

public:
  explicit MenuBar(QWidget* parent = 0);

  QAction* actionNew();
  QAction* actionOpen();
  QAction* actionSave();
  QAction* actionExit();
  QAction* actionExpertUser();

  void setActionExpertUser(bool expertUser);

protected:
  QMenu* menuOptions() const;

private:
  QMenu* menuOptions_;
  QAction* actionNew_;
  QAction* actionOpen_;
  QAction* actionSave_;
  QAction* actionExit_;
  QAction* actionExpertUser_;

};

} // namespace casaWizard
