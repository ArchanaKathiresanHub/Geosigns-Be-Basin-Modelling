//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//


// Interface for influential parameters
#pragma once

#include "argumentList.h"

namespace casaWizard
{

class ProjectReader;

namespace ua
{

class InfluentialParameter
{
public:

   enum class Type
   {
      TopCrustHeatProduction,
      TopCrustHeatProductionGrid,
      TopCrustHeatProductionGridScaling,
      ThermalConductivity,
      EquilibriumOceanicLithosphereThickness,
      InitialLithosphericMantleThickness,
      SurfaceTemperature
   };

   explicit InfluentialParameter(const QString& name, const QString& casaName, const QString& label, const QString& unitSI, const int totalNumberOfParameters, Type type);
   virtual ~InfluentialParameter() = default;
   int version() const;

   virtual QString label() const;
   virtual QString nameArgumentBased() const;
   QString casaName() const;
   QString name() const;
   QString labelName() const;
   QString unitSI() const;
   Type type() const;

   virtual void fillArguments(const ProjectReader& projectReader) = 0;

   int totalNumberOfParameters() const;
   ArgumentList& arguments();
   const ArgumentList& arguments() const;

   static InfluentialParameter* createFromString(const QString typeString, int version = 1);
   static InfluentialParameter* createFromIndex(int index, int version = 1);
   static InfluentialParameter* createFromList(const int version, const QStringList& list);
   static QStringList list();

   QStringList write() const;
   void read(const int version, const QString& input);

   void setMCMCResult(const QVector<double>& mcmcResult);
   double optimalValue() const;

protected:
   void addPDFArgument();

private:

   static Type typeFromString(const QString& typeString);
   static QString stringFromType(Type);

   static QVector<Type> types();
   static InfluentialParameter* create(const Type& type);

   ArgumentList m_arguments;
   QString m_name;
   QString m_casaName;
   QString m_labelName;
   QString m_unitSI;
   Type m_type;
   int m_totalNumberOfParameters;

   QVector<double> m_mcmcResult;

};

} // namespace ua

} // namespace casaWizard
