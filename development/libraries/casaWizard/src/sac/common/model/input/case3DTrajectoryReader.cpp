//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "case3DTrajectoryReader.h"

#include <QFile>
#include <QTextStream>

#include <assert.h>

namespace casaWizard
{

namespace sac
{

Case3DTrajectoryReader::Case3DTrajectoryReader(const QString filename) :
  m_filename{filename},
  m_properties{}
{
}

void Case3DTrajectoryReader::read()
{
  QFile file{m_filename};

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    throw std::runtime_error("Opening datafile " + m_filename.toStdString() + " failed");
  }

  QTextStream inputStream{&file};

  const QString header = inputStream.readLine();
  extractProperties(header);

  const int nProperties = m_properties.size();
  const int nDefaultColumns = 9;
  m_propertyValues.resize(nProperties);

  while (!inputStream.atEnd())
  {
    const QString line = inputStream.readLine();
    const QStringList lineSplit = line.split(",");

    m_x.append(lineSplit[0].toDouble());
    m_y.append(lineSplit[1].toDouble());
    m_depth.append(lineSplit[8].toDouble());

    for (int i = 0; i < nProperties; ++i)
    {
      m_propertyValues[i].append(lineSplit[i+nDefaultColumns].toDouble());
    }
  }
}

QStringList Case3DTrajectoryReader::properties() const
{
  return m_properties;
}

QVector<double> Case3DTrajectoryReader::x() const
{
  return m_x;
}

QVector<double> Case3DTrajectoryReader::y() const
{
  return m_y;
}

QVector<double> Case3DTrajectoryReader::depth() const
{
  return m_depth;
}

QVector<double> Case3DTrajectoryReader::readProperty(const QString property) const
{
  const int index = m_properties.indexOf(property);

  assert(m_properties.size() == m_propertyValues.size());
  if (index<0)
  {
    return {};
  }
  return m_propertyValues[index];
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

  m_properties = list;
}

}  // namespace sac

}  // namespace casaWizard

