#include "logger.h"

namespace casaWizard
{

Logger Logger::instance_ = Logger();
QString Logger::logMessage_ = "";

Logger::Logger() :
   QObject()
{
}

Logger& Logger::log()
{
   return instance_;
}

Logger& Logger::endl()
{
   instance_.logSignal(logMessage_);
   logMessage_.clear();
   return instance_;
}

Logger& Logger::operator<<(Logger& logger)
{
   return logger;
}

Logger& Logger::operator<<(const QString& message)
{
   logMessage_ += message;
   return instance_;
}

Logger& Logger::operator<<(const double& value)
{
   return instance_.operator<<(QString::number(value));
}

} // namespace casaWizard
