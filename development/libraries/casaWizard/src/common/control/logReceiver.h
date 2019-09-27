// Abstract base class for logging. The intention of this object is as follows:
// - The controller of the main program is derived from main controller, which has LogReceiver*
// - The constructor of the controller of the main program created the instantation of an object (of any derived class LogReceiver)
// - The derived class of LogReceiver implements the public slot log.
// - Objects that need to log have a signal log (trigger using emit), which is connected after the instantiation of that object (in a subcontroller).
#pragma once

#include <QObject>

class QString;

namespace casaWizard
{

class LogReceiver : public QObject
{
  Q_OBJECT

public:
  explicit LogReceiver(QObject* parent) : QObject(parent){}
  virtual void clear() = 0;

public slots:
  virtual void log(const QString& logMessage) = 0;
};

} // namespace casaWizard
