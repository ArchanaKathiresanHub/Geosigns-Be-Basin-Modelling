//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "influentialParameter.h"

#include "model/influentialParameter/equilibriumOceanicLithosphereThickness.h"
#include "model/influentialParameter/initialLithosphericMantleThickness.h"
#include "model/influentialParameter/surfaceTemperature.h"
#include "model/influentialParameter/thermalConductivity.h"
#include "model/influentialParameter/topCrustHeatProduction.h"
#include "model/influentialParameter/topCrustHeatProductionGrid.h"
#include "model/influentialParameter/topCrustHeatProductionGridScaling.h"

#include "model/input/projectReader.h"
#include "model/scenarioIO.h"

#include "ConstantsNumerical.h"
#include "Qt_Utils.h"

#include <QMap>

#include <assert.h>

namespace casaWizard
{

namespace ua
{

namespace
{
QMap<int,InfluentialParameter::Type> s_indexToTypeMapV0{{0,InfluentialParameter::Type::TopCrustHeatProduction},
                                                        {1,InfluentialParameter::Type::TopCrustHeatProductionGrid},
                                                        {2,InfluentialParameter::Type::TopCrustHeatProductionGridScaling},
                                                        {5,InfluentialParameter::Type::ThermalConductivity},
                                                        {6,InfluentialParameter::Type::EquilibriumOceanicLithosphereThickness},
                                                        {7,InfluentialParameter::Type::InitialLithosphericMantleThickness},
                                                        {8,InfluentialParameter::Type::SurfaceTemperature}};
}


InfluentialParameter::InfluentialParameter(const QString& name, const QString& casaName, const QString& label, const QString& unitSI, const int totalNumberOfParameters, Type type) :
   m_arguments{},
   m_name{name},
   m_casaName{casaName},
   m_labelName{label},
   m_unitSI{unitSI},
   m_type(type),
   m_totalNumberOfParameters{totalNumberOfParameters}
{
}

int InfluentialParameter::version() const
{
   return 1;
}

void InfluentialParameter::addPDFArgument()
{
   const QString pdfName{"Probability density function"};
   const QStringList pdfType{"Block", "Normal", "Triangle"};
   arguments().addArgument(pdfName, pdfType);
}

InfluentialParameter* InfluentialParameter::createFromString(const QString typeString, int /*version*/)
{
   return create(typeFromString(typeString));
}

InfluentialParameter* InfluentialParameter::createFromIndex(int index, int version)
{
   if (version == 0) //Needed because influential parameters crust thinning and crust thinning - one event (indices 3 and 4) in old version) were removed.
   {
      if (!s_indexToTypeMapV0.contains(index))
      {
         throw std::runtime_error("Error creating influential parameter.");
      }
      return create(s_indexToTypeMapV0[index]);
   }

   if (index >= types().size() || index < 0)
   {
      throw std::runtime_error("Error creating influential parameter");
   }
   return create(types()[index]);
}

InfluentialParameter* InfluentialParameter::createFromList(const int version, const QStringList& list)
{
   if (list.isEmpty())
   {
      return createFromIndex(0);
   }

   InfluentialParameter* ip = nullptr;
   if (version == 0)
   {
      const int index = list[0].toInt();
      ip = createFromIndex(index, version);
   }
   else
   {
      ip = createFromString(list[0]);
   }

   if (list.size() == 1)
   {
      return ip;
   }

   ip->read(version, list[1]);

   if (version >= 1 && list.size() > 2)
   {
      QVector<double> mcData;
      for (const QString& s : list[2].split(scenarioIO::subListSeparator))
      {
         if (s != "") //An empty string would be written as 0
         {
            mcData.append(s.toDouble());
         }

      }

      ip->setMCMCResult(mcData);
   }

   return ip;
}

InfluentialParameter* InfluentialParameter::create(const Type& type)
{

   switch (type)
   {
   case Type::TopCrustHeatProduction:
      return new TopCrustHeatProduction();
   case Type::TopCrustHeatProductionGrid:
      return new TopCrustHeatProductionGrid();
   case Type::TopCrustHeatProductionGridScaling:
      return new TopCrustHeatProductionGridScaling();
   case Type::ThermalConductivity:
      return new ThermalConductivity();
   case Type::EquilibriumOceanicLithosphereThickness:
      return new EquilibriumOceanicLithosphereThickness();
   case Type::InitialLithosphericMantleThickness:
      return new InitialLithosphericMantleThickness();
   case Type::SurfaceTemperature:
      return new SurfaceTemperature();
   default:
      return new TopCrustHeatProduction();
   }
}

QStringList InfluentialParameter::list()
{
   QStringList list;

   for (const Type& type : types())
   {
      InfluentialParameter* tmp = create(type);
      list.append(tmp->name());
      delete tmp;
   }

   return list;
}

QStringList InfluentialParameter::write() const
{
   QStringList mcmcData = qtutils::convertToQStringList(m_mcmcResult);

   QStringList out;
   out << stringFromType(type())
       << m_arguments.write()
       << mcmcData.join(scenarioIO::subListSeparator);

   return out;
}

void InfluentialParameter::read(const int version, const QString& input)
{
   m_arguments.read(version, input);
}

QVector<InfluentialParameter::Type> InfluentialParameter::types()
{
   return
   {
      Type::TopCrustHeatProduction,
            Type::TopCrustHeatProductionGrid,
            Type::TopCrustHeatProductionGridScaling,
            Type::ThermalConductivity,
            Type::EquilibriumOceanicLithosphereThickness,
            Type::InitialLithosphericMantleThickness,
            Type::SurfaceTemperature
   };
}

InfluentialParameter::Type InfluentialParameter::typeFromString(const QString& typeString)
{
   if (typeString == "TopCrustHeatProduction") return Type::TopCrustHeatProduction;
   if (typeString == "TopCrustHeatProductionGrid") return Type::TopCrustHeatProductionGrid;
   if (typeString == "TopCrustHeatProductionGridScaling") return Type::TopCrustHeatProductionGridScaling;
   if (typeString == "ThermalConductivity") return Type::ThermalConductivity;
   if (typeString == "EquilibriumOceanicLithosphereThickness") return Type::EquilibriumOceanicLithosphereThickness;
   if (typeString == "InitialLithosphericMantleThickness") return Type::InitialLithosphericMantleThickness;
   if (typeString == "SurfaceTemperature") return Type::SurfaceTemperature;
   throw std::runtime_error("Unknown influential parameter type string.");
}

QString InfluentialParameter::stringFromType(Type type)
{
   switch (type)
   {
   case Type::TopCrustHeatProduction: return "TopCrustHeatProduction";
   case Type::TopCrustHeatProductionGrid: return "TopCrustHeatProductionGrid";
   case Type::TopCrustHeatProductionGridScaling: return "TopCrustHeatProductionGridScaling";
   case Type::ThermalConductivity: return "ThermalConductivity";
   case Type::EquilibriumOceanicLithosphereThickness: return "EquilibriumOceanicLithosphereThickness";
   case Type::InitialLithosphericMantleThickness: return "InitialLithosphericMantleThickness";
   case Type::SurfaceTemperature: return "SurfaceTemperature";
   }
   assert(false); //Update the function with added types if this assert is reached.
   return "";
}

QString InfluentialParameter::label() const
{
   return QString(nameArgumentBased() + " " + m_unitSI);
}

QString InfluentialParameter::nameArgumentBased() const
{
   return name();
}

QString InfluentialParameter::casaName() const
{
   return m_casaName;
}

QString InfluentialParameter::name() const
{
   return m_name;
}

QString InfluentialParameter::labelName() const
{
   return m_labelName;
}

QString InfluentialParameter::unitSI() const
{
   return m_unitSI;
}

InfluentialParameter::Type InfluentialParameter::type() const
{
   return m_type;
}

int InfluentialParameter::totalNumberOfParameters() const
{
   return m_totalNumberOfParameters;
}

ArgumentList& InfluentialParameter::arguments()
{
   return m_arguments;
}

const ArgumentList& InfluentialParameter::arguments() const
{
   return m_arguments;
}

void InfluentialParameter::setMCMCResult(const QVector<double>& mcmcResult)
{
   m_mcmcResult = mcmcResult;
}

double InfluentialParameter::optimalValue() const
{
   if (m_mcmcResult.size() > 0)
   {
      return m_mcmcResult[0]; //Optimal value is the first one
   }
   return Utilities::Numerical::IbsNoDataValue;
}

} // namespace ua

} // namespace casaWizard

