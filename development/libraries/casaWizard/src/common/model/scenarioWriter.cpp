#include "scenarioWriter.h"

#include "writable.h"

#include <QFile>
#include <QVector>

namespace casaWizard
{

ScenarioWriter::ScenarioWriter(const QString& filename) :
  file_{filename},
  out_{&file_}
{
  file_.open(QFile::OpenModeFlag::WriteOnly);
}

casaWizard::ScenarioWriter::~ScenarioWriter()
{
  close();
}

void ScenarioWriter::writeValue(const QString& key, const char* value)
{
  writeValue(key, QString(value));
}

void ScenarioWriter::writeValue(const QString& key, const QString& value)
{
  out_ << formatLine(key, "string", value);
}

void ScenarioWriter::writeValue(const QString& key, const bool& value)
{
  out_ << formatLine(key, "bool", value?"1":"0");
}

void ScenarioWriter::writeValue(const QString& key, const int& value)
{
  out_ << formatLine(key, "int", QString::number(value));
}

void ScenarioWriter::writeValue(const QString& key, const double& value)
{
  out_ << formatLine(key, "double", scenarioIO::doubleToQString(value));
}

void ScenarioWriter::close()
{
  file_.close();
}

QString ScenarioWriter::formatLine(const QString& key, const QString& type, const QString& value)
{
  return QString(key + scenarioIO::separator
               + type + scenarioIO::separator
               + value + scenarioIO::newline);
}

template <>
QString ScenarioWriter::writeVectorEntry<bool>(const bool& value)
{
  return (value?"1":"0");
}

template <>
QString ScenarioWriter::writeVectorEntry<int>(const int& value)
{
  return QString::number(value);
}

template <>
QString ScenarioWriter::writeVectorEntry<double>(const double& value)
{
  return scenarioIO::doubleToQString(value);
}

template <>
QString ScenarioWriter::writeVectorEntry<QVector<double>>(const QVector<double>& value)
{
  QStringList list;
  for (const double d: value)
  {
    list << scenarioIO::doubleToQString(d);
  }
  return list.join(scenarioIO::listSeparator);
}

} // namespace casaWizard
