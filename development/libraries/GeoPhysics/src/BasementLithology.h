//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef _GEOPHYSICS_BASEMENTLITHOLOGY_H_
#define _GEOPHYSICS_BASEMENTLITHOLOGY_H_

#include "ConfigFileParameterAlc.h"
#include "SimpleLithology.h"

using namespace DataAccess;

namespace GeoPhysics {

class BasementLithology : public SimpleLithology
{
 public:


   BasementLithology ( DataAccess::Interface::ProjectHandle& projectHandle,
                       database::Record*                     record );


   virtual ~BasementLithology(){}

   double crustThermCondStandard (const double  inTemperature) const;
   double crustHeatCapStandard (const double inTemperature) const;
   double crustDensityStandard(const double  inTemperature, const double inLithostaticPressue) const;
   double crustThermCondLow (const double  inTemperature) const;
   double crustHeatCapLow (const double inTemperature) const;
   double crustDensityLow(const double  inTemperature, const double inLithostaticPressue) const;

   double mantleDensityStandard(const double  inTemperature, const double inLithostaticPressue) const ;
   double mantleHeatCapStandard (const double inTemperature) const;
   double mantleThermCondStandard (const double  inTemperature) const;
   double mantleThermCondLow (const double  inTemperature, const double inLithostaticPressue) const;
   double mantleHeatCapLow (const double inTemperature) const;
   double mantleDensityLow(const double  inTemperature, const double inLithostaticPressue) const;
   double mantleThermCondHigh (const double  inTemperature, const double inLithostaticPressue) const;
   double mantleHeatCapHigh (const double inTemperature) const;
   double mantleDensityHigh(const double  inTemperature, const double inLithostaticPressue) const;

   double densityBasalt(const double  inTemperature, const double inLithostaticPressue) const;
   double heatCapBasalt (const double inTemperature) const;
   double thermCondBasalt (const double inTemperature) const;

   double thermCondPointXu(const double inTemperature, const double inPressure) const;
   double thermCondPointWillis(const double inTemperature) const;

   double thermalconductivity(double t, double p) const;
   double basaltThermalConductivity(double t, double p) const;

   bool   setThermalModel( const std::string& aThermalModel );

   virtual double heatcapacity( const double t ) const;
   double getDensity( double t, double p ) const;
   double getBasaltDensity( double t, double p ) const;
   double getDensity( ) const;

   void setBasementLithoProperties( ConfigFileParameterAlc & aBP );
private:

   typedef enum {CRUST, MANTLE, BASALT, UNKNOWN} BasementLithologyType;

   BasementLithologyType  m_lithotype;
   ConfigFileParameterAlc m_constants; ///< Constants from the ALC configuration file

   void setLithoType();
private:

};
typedef BasementLithology* BasementLithologyPtr;

inline double GeoPhysics::BasementLithology::getDensity ()  const {
   return SimpleLithology::getDensity();
}

}

#endif /* _BASEMENTLITHO_H_ */
