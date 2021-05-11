#include "scenarioReader.h"

#include "model/logger.h"
#include "model/scenarioIO.h"

#include <QFile>
#include <QTextStream>
#include <QVector>

namespace casaWizard
{

ScenarioReader::ScenarioReader(const QString& filename) :
  bools_{},
  ints_{},
  doubles_{},
  strings_{},
  vectors_{}
{
  QFile file{filename};
  if (file.isOpen())
  {
    Logger::log() << "File is still open" << Logger::endl();
  }
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text ))
  {
    Logger::log() << "Failed to read " << filename << Logger::endl();
  }

  QTextStream in{&file};

  while (!in.atEnd())
  {
    const QString textLine{in.readLine()};
    const QStringList list = textLine.split(scenarioIO::separator);

    if (list[1] == "bool" )
    {
      bools_.insert(list[0], list[2]=="1");
    }
    else if (list[1] == "int")
    {
      ints_.insert(list[0], list[2].toInt());
    }
    else if (list[1] == "string")
    {
      strings_.insert(list[0], list[2]);
    }
    else if (list[1] == "double")
    {
      doubles_.insert(list[0], list[2].toDouble());
    }
    else if (list[1] == "vector")
    {
      const int nEntries = list[2].toInt();
      QStringList entries;
      for (int i=0; i<nEntries; ++i)
      {
        const QString entry{in.readLine()};
        QStringList l = entry.split(scenarioIO::separator);
        if (l[0] != list[0])
        {
          throw std::runtime_error("Unexpected line for reading " + list[1].toStdString());
        }
        l.removeFirst();
        entries.append(l.join(scenarioIO::separator));
      }
      vectors_.insert(list[0], entries);
    }
  }

  file.close();
}

QString ScenarioReader::readString(const QString& key) const
{
  return strings_.value(key, "");
}

bool ScenarioReader::readBool(const QString& key) const
{
  return bools_[key];
}

int ScenarioReader::readInt(const QString& key) const
{
  return ints_[key];
}

double ScenarioReader::readDouble(const QString& key) const
{
  return doubles_[key];
}

int ScenarioReader::extractVersion(QStringList& parameters) const
{
  if (parameters.isEmpty())
  {
    return -1;
  }
  const int version = parameters[0].toInt();
  parameters.removeFirst();
  return version;
}

template <>
bool ScenarioReader::createVectorEntry<bool>(const QString& entry) const
{
  return entry=="1";
}

template <>
int ScenarioReader::createVectorEntry<int>(const QString& entry) const
{
  return entry.toInt();
}

template <>
double ScenarioReader::createVectorEntry<double>(const QString& entry) const
{
  return entry.toDouble();
}

template <>
QVector<double> ScenarioReader::createVectorEntry<QVector<double>>(const QString& entry) const
{
  QVector<double> value;
  for (const QString& v : entry.split(scenarioIO::listSeparator))
  {
    if (!v.isEmpty())
    {
      value.append(v.toDouble());
    }
  }
  return value;
}

} // namespace casaWizard
