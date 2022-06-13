#include "predictionTarget.h"

#include "model/targetParameterMapCreator.h"
#include "model/PredictionTargetReader.h"

#include "StringHandler.h"

namespace casaWizard
{

namespace ua
{

PredictionTarget::PredictionTarget(const QVector<QString>& properties, const double x, const double y, const double age, const QString& locationName, const double z) :
  m_properties{properties},
  m_x{x},
  m_y{y},
  m_z{z},
  m_age{age},
  m_locationName(locationName)
{
}

PredictionTarget* PredictionTarget::createFromList(const int version, const QStringList& list)
{
   return PredictionTargetReader::readTarget(version, list);
}

QString PredictionTarget::casaCommandFromStrVec(std::vector<std::string> stringVec)
{
   //If strings contain spaces, add quotation marks:
   for (std::string& str : stringVec)
   {
      if (str.find_first_of(' ') != std::string::npos)
      {
         str = "\"" + str + "\"";
      }
   }

   //Concatenate with space as delimiter and return:
   return QString::fromStdString(StringHandler::implode(stringVec," "));
}

QString PredictionTarget::surfaceName() const
{
   return "";
}

double PredictionTarget::z() const
{
   return m_z;
}

void PredictionTarget::setZ(double z)
{
   m_z = z;
}

QVector<QString> PredictionTarget::properties() const
{
  return m_properties;
}

void PredictionTarget::setProperties(const QVector<QString>& properties)
{
   m_properties = properties;
}

void PredictionTarget::setPropertyActive(const bool active, const QString& property)
{
   if (active)
   {
      if (!m_properties.contains(property))
      {
         m_properties.append(property);
      }
   }
   else
   {
      m_properties.removeAll(property);
   }
}

double PredictionTarget::x() const
{
  return m_x;
}

void PredictionTarget::setX(double x)
{
  m_x = x;
}

double PredictionTarget::y() const
{
   return m_y;
}

void PredictionTarget::setY(double y)
{
   m_y = y;
}

double PredictionTarget::age() const
{
   return m_age;
}

void PredictionTarget::setAge(double age)
{
   m_age = age;
}

QString PredictionTarget::locationName() const
{
   return m_locationName;
}

void PredictionTarget::setLocationName(const QString& locationName)
{
   m_locationName = locationName;
}

} // namespace ua

} // namespace casaWizard
