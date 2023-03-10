#include "predictionTargetDepth.h"
#include "model/script/WizardDataToCasaScriptMapper.h"
#include "casaCmdInterface.h"

#include "model/scenarioIO.h"

#include "Qt_Utils.h"

namespace casaWizard
{

namespace ua
{

PredictionTargetDepth::PredictionTargetDepth(const QVector<QString>& properties, const double x, const double y, const double z, const double age, const QString& locationName) :
   PredictionTarget(properties, x, y, age, locationName, z)
{
}

PredictionTargetDepth PredictionTargetDepth::read(const int version, const QStringList& p)
{
   if (version > 0)
   {
      int numberOfProperties = p[1].toInt();
      QVector<QString> properties;
      for (int i = 0; i < numberOfProperties; i++)
      {
         properties.append(p[i + 2]);
      }
      return PredictionTargetDepth
      {
         properties,
               p[numberOfProperties + 2].toDouble(),
               p[numberOfProperties + 3].toDouble(),
               p[numberOfProperties + 4].toDouble(),
               p[numberOfProperties + 5].toDouble(),
               p[numberOfProperties + 6]
      };
   }
   else
   {
      if (p.size() != 5)
      {
         return PredictionTargetDepth{{"Unknown"}, 0, 0, 0, 0};
      }

      return PredictionTargetDepth
      {
         {p[0]},
         p[1].toDouble(),
               p[2].toDouble(),
               p[3].toDouble(),
               p[4].toDouble()
      };
   }
}

int PredictionTargetDepth::version() const
{
   return 1;
}

QStringList PredictionTargetDepth::write() const
{
   QStringList out;
   out << "depth";
   out << QString::number(properties().size());
   for (const QString& property : properties())
   {
      out << property;
   }
   out << scenarioIO::doubleToQString(x())
       << scenarioIO::doubleToQString(y())
       << scenarioIO::doubleToQString(z())
       << scenarioIO::doubleToQString(age())
       << locationName();

   return out;
}

QString PredictionTargetDepth::name(const QString& property) const
{
   auto doubleNoDigitToQString = [](double d){return QString::number(d, 'g', 12); };

   return property + " (" + doubleNoDigitToQString(x()) + ", " + doubleNoDigitToQString(y()) + ", " + doubleNoDigitToQString(z()) + ", " + QString::number(age(), 'f', 1) + ")";
}

QString PredictionTargetDepth::identifier(const QString& property) const
{
   std::vector<std::string> stringVec = identifierStringVec(property);
   std::string identifierString = casaCmdInterface::stringVecToStringWithNoSpaces(stringVec,"_");
   return QString::fromStdString(identifierString);
}

QString PredictionTargetDepth::casaCommand(const QString &property) const
{
   return casaCommandFromStrVec(identifierStringVec(property));
}

std::vector<std::string> PredictionTargetDepth::identifierStringVec(const QString& property) const
{
   using namespace wizardDataToCasaScriptMapper;

   std::vector<std::string> stringVec;
   stringVec.push_back(typeName().toStdString());
   stringVec.push_back(mapName(property).toStdString());
   stringVec.push_back(qtutils::doubleToQString(x()).toStdString());
   stringVec.push_back(qtutils::doubleToQString(y()).toStdString());
   stringVec.push_back(variable().toStdString());
   stringVec.push_back(qtutils::doubleOneDigitToQString(age()).toStdString());
   stringVec.push_back("1.0");
   stringVec.push_back("1.0");
   return stringVec;
}

QString PredictionTargetDepth::nameWithoutAge() const
{
   auto doubleNoDigitToQString = [](double d){return QString::number(d, 'g', 12); };
   QString propertiesString;
   for (const QString& property : properties())
   {
      propertiesString += "|" + property;
   }

   return propertiesString + " (" + doubleNoDigitToQString(x()) + ", " + doubleNoDigitToQString(y()) + ", " + doubleNoDigitToQString(z()) + ")";
}

QString PredictionTargetDepth::variable() const
{
   return QString::number(z(), 'g', 12);
}

QString PredictionTargetDepth::typeName() const
{
   return "XYZPoint";
}

PredictionTarget* PredictionTargetDepth::createCopy() const
{
   return new PredictionTargetDepth(*this);
}

} // namespace ua

} // namespace casaWizard
