#include "menuBar.h"

namespace casaWizard
{

MenuBar::MenuBar(QWidget* parent) :
  QMenuBar(parent),
  actionNew_{new QAction("New", this)},
  actionOpen_{new QAction("Open", this)},
  actionSave_{new QAction("Save", this)},
  actionExit_{new QAction("Exit", this)},
  menuOptions_{}
{
  QMenu* menuFile = addMenu("File");
  menuFile->addAction(actionNew_);
  menuFile->addAction(actionOpen_);
  menuFile->addAction(actionSave_);
  menuFile->addAction(actionExit_);

  actionExpertUser_ = new QAction("Expert user", this);
  actionExpertUser_->setCheckable(true);

  menuOptions_ = addMenu("Options");
  menuOptions_->addAction(actionExpertUser_);
}

QAction* MenuBar::actionNew()
{
  return actionNew_;
}

QAction* MenuBar::actionOpen()
{
  return actionOpen_;
}

QAction* MenuBar::actionSave()
{
  return actionSave_;
}

QAction* MenuBar::actionExit()
{
  return actionExit_;
}

QAction* MenuBar::actionExpertUser()
{
   return actionExpertUser_;
}

void MenuBar::setActionExpertUser(bool expertUser)
{
  actionExpertUser_->setChecked(expertUser);
}

QMenu* MenuBar::menuOptions() const
{
  return menuOptions_;
}

} // namespace casaWizard
