#include "logDisplayController.h"

#include "view/logDisplay.h"

#include <QPushButton>

#include <iostream>

namespace casaWizard
{

LogDisplayController::LogDisplayController(LogDisplay* logDisplay, QObject* parent) :
  LogReceiver(parent),
  logDisplay_{logDisplay}
{
  connect(logDisplay_->pushClearLog(), SIGNAL(clicked()), this, SLOT(slotPushClearLogClicked()));
  connect(logDisplay_->pushHideLog(), SIGNAL(clicked()), this, SLOT(slotPushHideLogClicked()));
  connect(logDisplay_->pushShowLog(), SIGNAL(clicked()), this, SLOT(slotPushShowLogClicked()));
}

void LogDisplayController::clear()
{
  logDisplay_->clearLog();
}

void LogDisplayController::log(const QString& logMessage)
{
  logDisplay_->addLog(logMessage);
  std::cout << logMessage.toStdString() << std::endl;
}

void LogDisplayController::slotPushClearLogClicked()
{
  logDisplay_->clearLog();
}

void LogDisplayController::slotPushShowLogClicked()
{
  logDisplay_->showMore();
}

void LogDisplayController::slotPushHideLogClicked()
{
  logDisplay_->showLess();
}

} // namespace casaWizard
