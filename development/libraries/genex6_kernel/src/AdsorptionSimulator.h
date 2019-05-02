#ifndef _GENEX6_KERNEL__ADSORPTION_SIMULATOR_H_
#define _GENEX6_KERNEL__ADSORPTION_SIMULATOR_H_

#include <string>

#include "Interface/ProjectHandle.h"

#include "SubProcessSimulator.h"
#include "SimulatorStateAdsorption.h"

#include "AdsorptionFunction.h"
#include "SimulatorStateBase.h"
#include "Input.h"
#include "ComponentManager.h"

namespace Genex6 {

   class SimulatorStateAdsorption;

   /// \brief Adsorption simulator.
   class AdsorptionSimulator : public SubProcessSimulator {

   public :

      static const unsigned int DefaultPriority = 100;

      /// \brief Constructor.
      AdsorptionSimulator ( DataAccess::Interface::ProjectHandle* projectHandle,
                            const bool isManaged );

      /// \brief Destructor.
      virtual ~AdsorptionSimulator ();

      /// \brief Perform the adsorption calculation.
      ///
      /// Some check are performed on the simulator-state.
      virtual void compute ( const Input&              sourceRockInput,
                                   SimulatorStateBase* baseState ) = 0;

      /// \brief Determine which of the species defined in the component manager are being considered for adsorption.
      virtual bool speciesIsSimulated ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const = 0;

      /// \brief Get the names of the species that is being adsorped.
      ///
      /// If the species is not a part of the adsorption simulation then a null string will be returned.
      virtual const std::string& getAdsorpedSpeciesName ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const = 0;

      /// \brief Get the names of the expelled species.
      ///
      /// If the species is not a part of the adsorption simulation then a null string will be returned.
      virtual const std::string& getExpelledSpeciesName ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const = 0;

      /// \brief Get the names of the free species.
      ///
      /// If the species is not a part of the adsorption simulation then a null string will be returned.
      virtual const std::string& getFreeSpeciesName ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const = 0;


      /// \brief Set a new adsorption-function.
      ///
      /// The adsorption-simulator object now takes control of the adsorption function.
      // Probably the adsorption-simulator's constructor should allocate the correct adsorption-function.
      virtual void setAdsorptionFunction ( AdsorptionFunction* newAdsorptionFunction );

      /// \brief return the objects project handle.
      virtual DataAccess::Interface::ProjectHandle* getProjectHandle () const;

      /// \brief Set process priority.
      void setPriority ( const unsigned int newPriority );

      /// \brief Get priority of adsorption process.
      unsigned int getPriority () const;

      bool managed () const;
      bool isGenex7() const;

      virtual void setSimulatorStateAdsorption( SimulatorStateAdsorption * state );
      double getOrganoPorosity() const;
      double getPessureLangmuir() const;
      double getPessurePrangmuir() const;
      double getPessureJGS() const;
      double getAdsGas() const;
      double getFreeGas() const;
      double getNmaxAds() const;
      double getNmaxFree() const;
      double getMolesGas() const;
      double getMolarGasVol() const;
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
      double getPL ( const unsigned int i,
                     const unsigned int j,
                     const double       temperature,
                     const double       toc,
                     const CBMGenerics::ComponentManager::SpeciesNamesId species ) const;

  
      double getVLReferenceTemperature () const;

   private :

      DataAccess::Interface::ProjectHandle* m_projectHandle;
      AdsorptionFunction*                        m_adsorptionFunction;
      unsigned int                               m_priority;
      bool                                       m_isManaged;

      SimulatorStateAdsorption * m_adsorptionCalculator;
      

   };

}

inline double Genex6::AdsorptionSimulator::getAdsorptionCapacity ( const unsigned int i,
                                                                   const unsigned int j,
                                                                   const double       temperature,
                                                                   const double       porePressure,
                                                                   const double       toc,
                                                                   const CBMGenerics::ComponentManager::SpeciesNamesId species ) const {

   return m_adsorptionFunction->compute ( i, j, temperature, porePressure, toc, species );
}

inline double Genex6::AdsorptionSimulator::getVL ( const unsigned int i,
                                                   const unsigned int j,
                                                   const double       temperature,
                                                   const double       toc,
                                                   const CBMGenerics::ComponentManager::SpeciesNamesId species ) const {

   return m_adsorptionFunction->computeVL ( i, j, temperature, toc, species );
}
inline double Genex6::AdsorptionSimulator::getPL ( const unsigned int i,
                                                   const unsigned int j,
                                                   const double       temperature,
                                                   const double       toc,
                                                   const CBMGenerics::ComponentManager::SpeciesNamesId species ) const {

   return m_adsorptionFunction->computePL ( i, j, temperature, toc, species );
}

inline double Genex6::AdsorptionSimulator::getVLReferenceTemperature () const {
   return m_adsorptionFunction->getReferenceTemperature ();
}

inline DataAccess::Interface::ProjectHandle* Genex6::AdsorptionSimulator::getProjectHandle () const {
   return m_projectHandle;
}

inline unsigned int Genex6::AdsorptionSimulator::getPriority () const {
   return m_priority;
}

inline bool Genex6::AdsorptionSimulator::managed () const {
   return m_isManaged;
}

inline bool Genex6::AdsorptionSimulator::isGenex7() const {

   return m_adsorptionCalculator != 0;

}


#endif // _GENEX6_KERNEL__ADSORPTION_SIMULATOR_H_
