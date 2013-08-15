#ifndef _BASEMENTLITHO_H_
#define _BASEMENTLITHO_H_

#include "SimpleLithology.h"

using namespace std;
using namespace DataAccess;

namespace GeoPhysics {

class BasementLithologyProps 
{
public:
   BasementLithologyProps();

   double m_csRho;
   double m_clRho;
   double m_bRho;
   double m_mRho;
   
   double m_csA;
   double m_csB;
   double m_clA;
   double m_clB;
   double m_bA;
   double m_bB;
   double m_mA;
   double m_mB;
   double m_bT;
   double m_bHeat;

   double m_HLmin;
   double m_NLMEmax;
   double m_HCBLmin;
   double m_InitMEmax;
   double m_ECTmin;

   BasementLithologyProps& operator=( const BasementLithologyProps& newBP );
   bool loadConfigurationFile(ifstream &ConfigurationFile );
private:
  void clean();

};
  

class BasementLithology : public SimpleLithology
{
 public:  


   BasementLithology ( DataAccess::Interface::ProjectHandle* projectHandle, 
                       database::Record*                     record );


   virtual ~BasementLithology(){};

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

   double thermCondPointHofmeister(const double inTemperature, const double inPressure) const; 
   double thermCondPointXu(const double inTemperature, const double inPressure) const; 
   double thermCondPointWillis(const double inTemperature) const; 

   double thermalconductivity(double t, double p) const;
   bool   setThermalModel( const string& aThermalModel );

   double heatcapacity( double t ) const;
   double getDensity( double t, double p ) const;
   double getDensity( ) const;

   void setBasementLithoProperties( BasementLithologyProps & aBP );
private:
 
   typedef enum {CRUST, MANTLE, BASALT, UNKNOWN} BasementLithologyType;

   // typedef double (*fn) ( double, double );
 
   // fn * thermfuncs [3];
   BasementLithologyType m_lithotype;

   BasementLithologyProps m_constants;

   void setLithoType();
private:

};
typedef BasementLithology* BasementLithologyPtr;

inline double GeoPhysics::BasementLithology::getDensity ()  const {
   return SimpleLithology::getDensity();
}

}

#endif /* _BASEMENTLITHO_H_ */
