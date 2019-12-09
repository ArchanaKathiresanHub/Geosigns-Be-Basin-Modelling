#ifndef _GENEX6_KERNEL__LANGMUIR_ADSORPTION_FNUCTION_H_
#define _GENEX6_KERNEL__LANGMUIR_ADSORPTION_FNUCTION_H_

#include <string>

#include "ProjectHandle.h"

#include "AdsorptionFunction.h"
#include "ComponentManager.h"

#include "LangmuirAdsorptionIsothermInterval.h"


namespace Genex6 {

   /// \brief Identifier needed to determine whith adsorption function is to be allocated in the factory.
   const std::string LangmuirAdsorptionFunctionId = "LangmuirAdsorptionFunction";

   /// \brief Allocates an adsorption function.
   AdsorptionFunction* allocateLangmuirAdsorptionFunction ( DataAccess::Interface::ProjectHandle& projectHandle,
                                                            const std::string&                         langmuirName );

   /// A simple Langmuir adsorption function for C1.
   class LangmuirAdsorptionFunction : public Genex6::AdsorptionFunction {

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

      LangmuirAdsorptionFunction ( DataAccess::Interface::ProjectHandle& projectHandle,
                                   const std::string&                         formationName );

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

      /// \brief Indicate whether or not the adsorption function has valid values.
      bool isValid () const;

      /// \brief Get information about any error that may have occurred during initialisation.
      std::string getErrorMessage () const;

   private :

      LangmuirAdsorptionIsothermIntervalVector m_isothermIntervals;

      double m_temperatureLowerBound;
      double m_temperatureUpperBound;
      mutable LangmuirAdsorptionIsothermInRange m_temperatureInRange;
      std::string m_langmuirName;

   };

}

#endif // _GENEX6_KERNEL__LANGMUIR_ADSORPTION_FNUCTION_H_
