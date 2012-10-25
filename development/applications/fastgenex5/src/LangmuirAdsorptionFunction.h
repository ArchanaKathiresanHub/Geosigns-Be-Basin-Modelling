#ifndef _GENEX5_SIMULATOR__LANGMUIR_ADSORPTION_FNUCTION_H_
#define _GENEX5_SIMULATOR__LANGMUIR_ADSORPTION_FNUCTION_H_

#include <string>


// #include "database.h"

#include "AdsorptionFunction.h"
#include "ComponentManager.h"

#include "AdsorptionProjectHandle.h"
#include "LangmuirAdsorptionIsothermInterval.h"

// class AdsorptionProjectHandle;


const std::string LangmuirAdsorptionFunctionId = "LangmuirAdsorptionFunction";

Genex5::AdsorptionFunction* allocateLangmuirAdsorptionFunction ( AdsorptionProjectHandle* projectHandle,
                                                                 const std::string&       langmuirName );

/// A simple Langmuir adsorption function for C1.
class LangmuirAdsorptionFunction : public Genex5::AdsorptionFunction {

   /// Functor class for 
   class LangmuirAdsorptionIsothermLowerBoundOrder {

   public :

      bool operator ()( const LangmuirAdsorptionIsothermInterval* i1,
                        const LangmuirAdsorptionIsothermInterval* i2 ) const;

   };

   /// Functor class for indicating if a set temperature is in the range of the interval. 
   class LangmuirAdsorptionIsothermInRange {

   public :

      LangmuirAdsorptionIsothermInRange ();

      /// Set the desired temperature.
      void setTemperature ( const double temperature );

      bool operator ()( const LangmuirAdsorptionIsothermInterval* interval ) const;

   private :

      double m_temperature;

   };


   typedef std::vector<LangmuirAdsorptionIsothermInterval*> LangmuirAdsorptionIsothermIntervalVector;

public :

   LangmuirAdsorptionFunction ( AdsorptionProjectHandle* adsorptionProjectHandle,
                                const std::string& formationName );

   ~LangmuirAdsorptionFunction () {}

   /// Compute the adsorption capacity.
   ///
   /// Temperature in Celcius, pressure in MPa.
   double compute ( const unsigned int i,
                    const unsigned int j,
                    const double       temperature,
                    const double       porePressure,
                    const double       toc,
                    const CBMGenerics::ComponentManager::SpeciesNamesId species ) const;

   /// Compute the VL.
   double computeVL ( const unsigned int i,
                      const unsigned int j,
                      const double       temperature,
                      const double       toc,
                      const CBMGenerics::ComponentManager::SpeciesNamesId species ) const;

   double getReferenceTemperature () const;

   void initialise ();

private :

   AdsorptionProjectHandle* m_adsorptionProjectHandle;

   LangmuirAdsorptionIsothermIntervalVector m_isothermIntervals;

   double m_temperatureLowerBound;
   double m_temperatureUpperBound;
   mutable LangmuirAdsorptionIsothermInRange m_temperatureInRange;
   std::string m_langmuirName;

};

#endif // _GENEX5_SIMULATOR__LANGMUIR_ADSORPTION_FNUCTION_H_
