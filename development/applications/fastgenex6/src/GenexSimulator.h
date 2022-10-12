#ifndef _GENEXSIMULATION_GENEXSIMULATOR_H_
#define _GENEXSIMULATION_GENEXSIMULATOR_H_

#include <string>
#include <vector>
#include "petsc.h"

#include "ProjectFileHandler.h"

namespace database
{
   class Database;
}
#include "Interface.h"
#include "DerivedPropertyManager.h"

#include "GenexSourceRock.h"

using namespace DataAccess;


namespace GenexSimulation
{
   const std::string GenexActivityName = "Genex5";

   //! GenexSimulator Controls the execution of Generation and Expulsion simulation.
   /*!
      Derives from Interface::ProjectHandle
   */
   class GenexSimulator : public GeoPhysics::ProjectHandle
   {
   public:
      //! Constructor
      /*!
         \param database a pointer to a database::Database object
         \param name the name of the file
      */
      GenexSimulator (database::ProjectFileHandlerPtr database, const std::string & name, const DataAccess::Interface::ObjectFactory* objectFactory);

      static GenexSimulator *CreateFrom (const std::string & inputFileName, DataAccess::Interface::ObjectFactory* objectFactory);

      virtual ~GenexSimulator (void);

      /// Invokes the simulation execution
      bool run();

      /// Save results
      bool saveTo (const std::string & outputFileName);

      /// Delete property values associated with generation and expulsion simulation
      void deleteSourceRockPropertyValues();

      /// megre separate opuput files into a one
      bool mergeOutputFiles ();

   private:
      /// Sets the requested output properties according to Filter Time IO and fastmig requirements
      void setRequestedOutputProperties();

      /// Sets the requested output properties related to Species output
      void setRequestedSpeciesOutputProperties();
      //! Registers the full set of available output properties
      /*!
            Details of registerProperties
      */
      void registerProperties();
      /// Check if  propertyName is among the fastgenex5 registered properties
      bool isPropertyRegistered(const std::string & propertyName);


      DerivedProperties::DerivedPropertyManager* m_propertyManager;

      std::vector<std::string> m_registeredProperties;
      std::vector<std::string> m_shaleProperties;
      std::vector<std::string> m_requestedProperties;
      std::vector<std::string> m_expelledToCarrierBedProperties;
      std::vector<std::string> m_expelledToSourceRockProperties;
      std::vector<std::string> m_expelledToCarrierBedPropertiesS;
      std::vector<std::string> m_expelledToSourceRockPropertiesS;

      bool computeSourceRock (Genex6::GenexSourceRock * aSourceRock, const Interface::Formation * aFormation );

   };

}

#endif
