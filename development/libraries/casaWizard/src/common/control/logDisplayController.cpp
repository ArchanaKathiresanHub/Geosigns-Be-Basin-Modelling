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

} // namespace casaWizard
