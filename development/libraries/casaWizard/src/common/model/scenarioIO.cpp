#include "scenarioIO.h"

#include <QStringList>
#include <QVector>

namespace casaWizard
{

namespace scenarioIO
{

QString vectorToWrite(const QVector<double>& vector)
{
  QStringList list;
  for (const double d : vector)
  {
    list << doubleToQString(d);
  }
  return list.join(listSeparator);
}

QVector<double> vectorFromRead(const QString& string)
{
  QVector<double> vector;
  if (string.isEmpty())
  {
    return vector;
  }

  QStringList list = string.split(listSeparator);
  for (const QString& entry : list)
  {
    vector.push_back(entry.toDouble());
  }
  return vector;
}

QString doubleToQString(const double d)
{
  return QString::number(d, 'f');
}

} // namespace scenarioIO

} // namespace casaWizard
