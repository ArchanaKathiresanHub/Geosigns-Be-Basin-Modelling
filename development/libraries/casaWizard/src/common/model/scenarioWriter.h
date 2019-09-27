// Writer for scenario objects
#pragma once

#include "scenarioIO.h"

#include <QFile>
#include <QStringList>
#include <QTextStream>

namespace casaWizard
{

class ScenarioWriter
{
public:
  explicit ScenarioWriter(const QString& filename);
  ~ScenarioWriter();

  void writeValue(const QString& key, const char* value);
  void writeValue(const QString& key, const QString& value);
  void writeValue(const QString& key, const bool& value);
  void writeValue(const QString& key, const int& value);
  void writeValue(const QString& key, const double& value);

  template<typename Type>
  void writeValue(const QString& key, const QVector<Type>& values);

  void close();

private:
  template<typename Type>
  QString writeVectorEntry(const Type& value);

  QString formatLine(const QString& key, const QString& type, const QString& value);
  QFile file_;
  QTextStream out_;
};

// These functions are  used for the templated write function, such that it is guaranteed to act on a pointer type
template <typename Type>
Type* pointer(Type& object)
{
  return &object;
}

template <typename Type>
Type* pointer(Type* object)
{
  return object;
}

template <typename Type>
void ScenarioWriter::writeValue(const QString& key, const QVector<Type>& values)
{
  out_ << formatLine(key, "vector", QString::number(values.size()));
  for (const Type& value: values)
  {
    out_ << key << scenarioIO::separator
         << writeVectorEntry(value) << scenarioIO::newline;
  }
}

template<typename Type>
QString ScenarioWriter::writeVectorEntry(const Type& value)
{
  return QString::number(pointer(value)->version())
      + scenarioIO::separator
      + pointer(value)->write().join(scenarioIO::separator);
}

template <> QString ScenarioWriter::writeVectorEntry<bool>(const bool& value);
template <> QString ScenarioWriter::writeVectorEntry<int>(const int& value);
template <> QString ScenarioWriter::writeVectorEntry<double>(const double& value);
template <> QString ScenarioWriter::writeVectorEntry<QVector<double>>(const QVector<double>& value);

} // namespace casaWizard
