#include "case3DTrajectoryReader.h"

#include <QFile>
#include <QTextStream>

#include <assert.h>

namespace casaWizard
{

namespace sac
{

Case3DTrajectoryReader::Case3DTrajectoryReader(const QString filename) :
  filename_{filename},
  properties_{}
{
}

void Case3DTrajectoryReader::read()
{
  QFile file{filename_};

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    throw std::runtime_error("Opening datafile " + filename_.toStdString() + " failed");
  }

  QTextStream inputStream{&file};

  const QString header = inputStream.readLine();
  extractProperties(header);

  const int nProperties = properties_.size();
  const int nDefaultColumns = 9;
  propertyValues_.resize(nProperties);

  while (!inputStream.atEnd())
  {
    const QString line = inputStream.readLine();
    const QStringList lineSplit = line.split(",");

    x_.append(lineSplit[0].toDouble());
    y_.append(lineSplit[1].toDouble());
    depth_.append(lineSplit[8].toDouble());

    for (int i = 0; i < nProperties; ++i)
    {
      propertyValues_[i].append(lineSplit[i+nDefaultColumns].toDouble());
    }
  }
}

QStringList Case3DTrajectoryReader::properties() const
{
  return properties_;
}

QVector<double> Case3DTrajectoryReader::x() const
{
  return x_;
}

QVector<double> Case3DTrajectoryReader::y() const
{
  return y_;
}

QVector<double> Case3DTrajectoryReader::depth() const
{
  return depth_;
}

QVector<double> Case3DTrajectoryReader::readProperty(const QString property) const
{
  const int index = properties_.indexOf(property);

  assert(properties_.size() == propertyValues_.size());
  if (index<0)
  {
    return {};
  }
  return propertyValues_[index];
}

void Case3DTrajectoryReader::extractProperties(const QString header)
{
  QStringList list = header.split(",");

  // Skip first 9 entries
  // X(m),Y(m),I,J,Age(Ma),Formation,Surface,LayerIndex,Depth(m)
  list.erase(list.begin(), list.begin()+9);

  for (QString& prop : list)
  {
    prop = prop.split("(")[0];
  }

  properties_ = list;
}

}  // namespace sac

}  // namespace casaWizard

