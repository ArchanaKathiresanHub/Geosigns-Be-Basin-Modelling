#ifndef _GENEX5__ADSORPTION_SIMULATOR_H_
#define _GENEX5__ADSORPTION_SIMULATOR_H_

#include <string>

#include "AdsorptionFunction.h"
#include "SimulatorState.h"
#include "SourceRockNodeInput.h"
#include "SourceRockNodeOutput.h"
#include "ComponentManager.h"

namespace Genex5 {


   class AdsorptionSimulator {

   public :

      AdsorptionSimulator ();

      virtual ~AdsorptionSimulator ();

      /// Perform the adsorption calculation.
      virtual void compute ( const double                thickness,
                             const double                inorganicDensity,
                             const SourceRockNodeInput&  sourceRockInput,
                                   SourceRockNodeOutput& sourceRockOutput,
                                   SimulatorState*       simulatorState ) = 0;

      /// Determine which of the species defined in the component manager are being considered for adsorption.
      virtual bool speciesIsSimulated ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const = 0;

      /// Get the names of the species that is being adsorped.
      ///
      /// If the species is not a part of the adsorption simulation then a null string will be returned.
      virtual const std::string& getAdsorpedSpeciesName ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const = 0;

      /// Get the names of the expelled species.
      ///
      /// If the species is not a part of the adsorption simulation then a null string will be returned.
      virtual const std::string& getExpelledSpeciesName ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const = 0;

      /// Get the names of the free species.
      ///
      /// If the species is not a part of the adsorption simulation then a null string will be returned.
      virtual const std::string& getFreeSpeciesName ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const = 0;


      /// Set a new adsorption-function.
      ///
      /// The adsorption-simulator object now takes control of the adsorption function.
      // Probably the adsorption-simulator's constructor should allocate the correct adsorption-function.
      virtual void setAdsorptionFunction ( AdsorptionFunction* newAdsorptionFunction );


   protected :

      double getAdsorptionCapacity ( const unsigned int i,
                                     const unsigned int j,
                                     const double       temperature,
                                     const double       porePressure,
                                     const double       toc,
                                     const CBMGenerics::ComponentManager::SpeciesNamesId species ) const;

      double getVL ( const unsigned int i,
                     const unsigned int j,
                     const double       temperature,
                     const double       toc,
                     const CBMGenerics::ComponentManager::SpeciesNamesId species ) const;

      double getVLReferenceTemperature () const;

   private :

      AdsorptionFunction* m_adsorptionFunction;

   };

}

inline double Genex5::AdsorptionSimulator::getAdsorptionCapacity ( const unsigned int i,
                                                                   const unsigned int j,
                                                                   const double       temperature,
                                                                   const double       porePressure,
                                                                   const double       toc,
                                                                   const CBMGenerics::ComponentManager::SpeciesNamesId species ) const {

   return m_adsorptionFunction->compute ( i, j, temperature, porePressure, toc, species );
}

inline double Genex5::AdsorptionSimulator::getVL ( const unsigned int i,
                                                   const unsigned int j,
                                                   const double       temperature,
                                                   const double       toc,
                                                   const CBMGenerics::ComponentManager::SpeciesNamesId species ) const {

   return m_adsorptionFunction->computeVL ( i, j, temperature, toc, species );
}

inline double Genex5::AdsorptionSimulator::getVLReferenceTemperature () const {
   return m_adsorptionFunction->getReferenceTemperature ();
}


#endif // _GENEX5__ADSORPTION_SIMULATOR_H_
