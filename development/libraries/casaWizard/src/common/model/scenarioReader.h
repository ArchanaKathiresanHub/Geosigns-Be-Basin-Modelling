// Reader for scenario objects
#pragma once

#include "scenarioIO.h"

#include <QMap>
#include <QString>

namespace casaWizard
{

class ScenarioReader
{
public:
  explicit ScenarioReader(const QString& filename);

  QString readString(const QString& key) const;
  bool readBool(const QString& key) const;
  int readInt(const QString& key) const;
  double readDouble(const QString& key) const;

  template <typename Type>
  QVector<Type> readVector(const QString& key) const;

  template <typename Type>
  QVector<Type*> readAndCreateVector(const QString& key) const;

private:
  template<typename Type>
  Type createVectorEntry(const QString& entry) const;

  int extractVersion(QStringList& parameters) const;

  QMap<QString, bool> bools_;
  QMap<QString, int> ints_;
  QMap<QString, double> doubles_;
  QMap<QString, QString> strings_;
  QMap<QString, QStringList> vectors_;
};

template<typename Type>
QVector<Type> ScenarioReader::readVector(const QString& key) const
{
  QVector<Type> values;
  for (const QString& entry : vectors_[key])
  {
    if (!entry.isEmpty())
    {

      values.append(createVectorEntry<Type>(entry));
    }
  }
  return values;
}

template<typename Type>
QVector<Type*> ScenarioReader::readAndCreateVector(const QString& key) const
{
  QVector<Type*> values;
  for (const QString& entry : vectors_[key] )
  {
    if (!entry.isEmpty())
    {
      QStringList parameters = entry.split(scenarioIO::separator);
      const int version = extractVersion(parameters);
      Type* t(Type::createFromList(version, parameters));
      values.append(t);
    }
  }
  return values;
}

template<typename Type>
Type ScenarioReader::createVectorEntry(const QString& entry) const
{
  QStringList parameters = entry.split(scenarioIO::separator);
  const int version = extractVersion(parameters);
  return Type::read(version, parameters);
}

template<> bool ScenarioReader::createVectorEntry<bool>(const QString& entry) const;
template<> int ScenarioReader::createVectorEntry<int>(const QString& entry) const;
template<> double ScenarioReader::createVectorEntry<double>(const QString& entry) const;
template<> QVector<double> ScenarioReader::createVectorEntry<QVector<double>>(const QString& entry) const;

} // namespace casaWizard
