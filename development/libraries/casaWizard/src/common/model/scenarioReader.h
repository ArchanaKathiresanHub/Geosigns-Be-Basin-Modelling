// Reader for scenario objects
#pragma once

#include "scenarioIO.h"

#include <QMap>
#include <QString>

#include <memory>

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

  template <typename Type, typename Type2>
  QMap<Type, Type2> readMap(const QString& key) const;


  template <typename Type>
  QVector<Type*> readAndCreateVector(const QString& key) const;

  template<typename Type>
  QVector<std::shared_ptr<Type>> readAndCreateVectorOfSharedPtrs(const QString& key) const;
private:
  template<typename Type>
  Type createEntry(const QString& entry) const;

  int extractVersion(QStringList& parameters) const;

  QMap<QString, bool> bools_;
  QMap<QString, int> ints_;
  QMap<QString, double> doubles_;
  QMap<QString, QString> strings_;
  QMap<QString, QStringList> vectors_;
  QMap<QString, QMap<QString, QString>> maps_;
};

template<typename Type>
QVector<Type> ScenarioReader::readVector(const QString& key) const
{
  QVector<Type> values;
  for (const QString& entry : vectors_[key])
  {
    if (!entry.isEmpty())
    {

      values.append(createEntry<Type>(entry));
    }
  }
  return values;
}

template<typename Type, typename Type2>
QMap<Type, Type2> ScenarioReader::readMap(const QString& key1) const
{
  QMap<Type, Type2> valuesTest;
  for (const auto& mapKey : maps_[key1].keys())
  {
    valuesTest.insert(createEntry<Type>(mapKey), createEntry<Type2>(maps_[key1][mapKey]));
  }
  return valuesTest;
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
QVector<std::shared_ptr<Type>> ScenarioReader::readAndCreateVectorOfSharedPtrs(const QString& key) const
{
  QVector<std::shared_ptr<Type>> values;
  for (const QString& entry : vectors_[key] )
  {
    if (!entry.isEmpty())
    {
      QStringList parameters = entry.split(scenarioIO::separator);
      const int version = extractVersion(parameters);
      values.append(std::shared_ptr<Type>(Type::createFromList(version, parameters)));
    }
  }
  return values;
}

template<typename Type>
Type ScenarioReader::createEntry(const QString& entry) const
{
  QStringList parameters = entry.split(scenarioIO::separator);
  const int version = extractVersion(parameters);
  return Type::read(version, parameters);
}

template<> bool ScenarioReader::createEntry<bool>(const QString& entry) const;
template<> int ScenarioReader::createEntry<int>(const QString& entry) const;
template<> double ScenarioReader::createEntry<double>(const QString& entry) const;
template<> QString ScenarioReader::createEntry<QString>(const QString& entry) const;
template<> QVector<double> ScenarioReader::createEntry<QVector<double>>(const QString& entry) const;

} // namespace casaWizard
