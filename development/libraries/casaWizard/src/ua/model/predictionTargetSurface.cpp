#include "predictionTargetSurface.h"
#include "model/ToDepthConverter.h"
#include "model/scenarioIO.h"
#include "casaCmdInterface.h"
#include "model/script/WizardDataToCasaScriptMapper.h"

#include "Qt_Utils.h"

#include <regex>

namespace casaWizard
{

namespace ua
{

PredictionTargetSurface::PredictionTargetSurface(const QVector<QString>& properties, const double x, const double y, const QString& surface, const double age, const ToDepthConverter* todepthConverter, const QString& locationName) :
  PredictionTarget(properties, x, y, age, locationName),
  m_surface{surface},
  m_toDepthConverter{todepthConverter}
{
   if (m_toDepthConverter)
   {
      setZ(m_toDepthConverter->getDepth(x,y,surface));
   }

}

PredictionTargetSurface PredictionTargetSurface::read(const int version, const QStringList& p)
{
   if (version > 0)
   {
      int numberOfProperties = p[1].toInt();
      QVector<QString> properties;
      for (int i = 0; i < numberOfProperties; i++)
      {
         properties.append(p[i + 2]);
      }
      return PredictionTargetSurface
      {
         properties,
         p[numberOfProperties + 2].toDouble(),
         p[numberOfProperties + 3].toDouble(),
         p[numberOfProperties + 4],
         p[numberOfProperties + 5].toDouble(),
         nullptr,
         p[numberOfProperties + 7]
      };
   }
   else
   {
      if (p.size() != 5)
      {
       return PredictionTargetSurface{{"Unknown"}, 0, 0, "", 0};
      }

      return PredictionTargetSurface
      {
        {p[0]},
        p[1].toDouble(),
        p[2].toDouble(),
        p[3],
        p[4].toDouble()
      };
   }
}

int PredictionTargetSurface::version() const
{
  return 1;
}

QStringList PredictionTargetSurface::write() const
{
  QStringList out;
  out << "surface";
  out << QString::number(properties().size());
  for (const QString& property : properties())
  {
     out << property;
  }
  out << scenarioIO::doubleToQString(x())
      << scenarioIO::doubleToQString(y())
      << m_surface
      << scenarioIO::doubleToQString(age())
      << scenarioIO::doubleToQString(z())
      << locationName();

  return out;
}

QString PredictionTargetSurface::name(const QString& property) const
{
  auto doubleNoDigitToQString = [](double d){return QString::number(d, 'g', 12); };

  return property + " (" + doubleNoDigitToQString(x()) + ", " + doubleNoDigitToQString(y()) + ", " + m_surface + ", " + QString::number(age(), 'f', 1) + ")";
}

QString PredictionTargetSurface::identifier(const QString& property) const
{
   std::vector<std::string> stringVec = identifierStringVec(property);
   std::string identifierString = casaCmdInterface::stringVecToStringWithNoSpaces(stringVec,"_");
   return QString::fromStdString(identifierString);
}

QString PredictionTargetSurface::casaCommand(const QString& property) const
{
   return casaCommandFromStrVec(identifierStringVec(property));
}

std::vector<std::string> PredictionTargetSurface::identifierStringVec(const QString& property) const
{
   using namespace wizardDataToCasaScriptMapper;

   std::vector<std::string> stringVec;
   stringVec.push_back(typeName().toStdString());
   stringVec.push_back(mapName(property).toStdString());
   stringVec.push_back(qtutils::doubleToQString(x()).toStdString());
   stringVec.push_back(qtutils::doubleToQString(y()).toStdString());
   stringVec.push_back(m_layer.toStdString());
   stringVec.push_back(qtutils::doubleOneDigitToQString(age()).toStdString());
   stringVec.push_back("1.0");
   stringVec.push_back("1.0");
   return stringVec;
}

QString PredictionTargetSurface::nameWithoutAge() const
{
  auto doubleNoDigitToQString = [](double d){return QString::number(d, 'g', 12); };
  QString propertiesString;
  for (const QString& property : properties())
  {
     propertiesString += "|" + property;
  }
  return propertiesString + " (" + doubleNoDigitToQString(x()) + ", " + doubleNoDigitToQString(y()) + ", " + m_surface + ")";
}

void PredictionTargetSurface::setToDepthConverterAndCalcDepth(const ToDepthConverter* todepthConverter)
{
   m_toDepthConverter = todepthConverter;
   if (m_toDepthConverter)
   {
      setZ(m_toDepthConverter->getDepth(x(),y(),surfaceName()));
   }
}

void PredictionTargetSurface::setX(double x)
{
   PredictionTarget::setX(x);

   if (m_toDepthConverter)
   {
      setZ(m_toDepthConverter->getDepth(x,y(),surfaceName()));
   }
}

void PredictionTargetSurface::setY(double y)
{
   PredictionTarget::setY(y);

   if (m_toDepthConverter)
   {
      setZ(m_toDepthConverter->getDepth(x(),y,surfaceName()));
   }
}

QString PredictionTargetSurface::variable() const
{
  return "\"" + m_layer + "\"";
}

QString PredictionTargetSurface::typeName() const
{
   return "XYPointLayerTopSurface";
}

QString PredictionTargetSurface::surfaceName() const
{
   return m_surface;
}

void PredictionTargetSurface::setSurfaceAndLayerName(const QString& surfaceName, const QString layerName)
{
   m_surface = surfaceName;
   m_layer = layerName;
   if (m_toDepthConverter)
   {
      setZ(m_toDepthConverter->getDepth(x(),y(),m_surface));
   }
}

PredictionTarget* PredictionTargetSurface::createCopy() const
{
   return new PredictionTargetSurface(*this);
}

} // namespace ua

} // namespace casaWizard
