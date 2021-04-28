// Controller for the display of the log.
// The display is a text field with a clear button
#pragma once

#include "logReceiver.h"

namespace casaWizard
{

class LogDisplay;

class LogDisplayController : public LogReceiver
{
  Q_OBJECT

public:
  explicit LogDisplayController(LogDisplay* logDisplay, QObject* parent);
  void clear() override;

public slots:
  void log(const QString &logMessage) override;

private slots:
  void slotPushClearLogClicked();
  void slotPushShowLogClicked();
  void slotPushHideLogClicked();

private:
  LogDisplay* logDisplay_;

};

} // namespace casaWizard
