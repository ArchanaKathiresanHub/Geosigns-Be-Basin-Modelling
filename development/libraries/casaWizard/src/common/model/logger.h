// Logger class
#pragma once

#include <QObject>
#include <QString>

namespace casaWizard
{

class Logger : public QObject
{
  Q_OBJECT

public:
  static Logger& log();
  static Logger& endl();

  Logger& operator<<(const QString& logMessage);
  Logger& operator<<(const double& value);
  Logger& operator<<(Logger& logger);

signals:
  void logSignal(const QString& logMessage);

private:
  Logger();
  void operator=(const Logger&) = delete;
  Logger(const Logger&) = delete;

  static Logger instance_;
  static QString logMessage_;
};

} // namespace casaWizard
