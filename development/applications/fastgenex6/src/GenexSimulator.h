#ifndef _GENEXSIMULATION_GENEXSIMULATOR_H_
#define _GENEXSIMULATION_GENEXSIMULATOR_H_

#include <string>
#include <vector>
#include "petsc.h"
namespace database
{
   class Database;
}
#include "Interface/Interface.h"
#include "Interface/ProjectHandle.h"

#include "SourceRock.h"

using namespace DataAccess;


namespace GenexSimulation
{
   const string GenexActivityName = "Genex5";

   //! GenexSimulator Controls the execution of Generation and Expulsion simulation.
   /*! 
      Derives from Interface::ProjectHandle
   */
   class GenexSimulator : public Interface::ProjectHandle
   {
   public:
      //! Constructor
      /*!
         \param database a pointer to a database::Database object
         \param name the name of the file
         \param accessMode read or write
      */
      GenexSimulator (database::Database * database, const std::string & name, const std::string & accessMode);
      
      static GenexSimulator *CreateFrom (const std::string & inputFileName);

      virtual ~GenexSimulator (void);

      /// Invokes the simulation execution
      bool run();

      /// Save results
      bool saveTo (const std::string & outputFileName);

      /// Delete property values associated with generation and expulsion simulation
      void deleteSourceRockPropertyValues();

      /// megre separate opuput files into a one
      bool mergeOutputFiles ();
   protected:

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
      bool isPropertyRegistered(const string & propertyName);

   private:

      vector<string> m_registeredProperties;
      vector<string> m_shaleProperties;
      vector<string> m_requestedProperties;
      vector<string> m_expelledToCarrierBedProperties;
      vector<string> m_expelledToSourceRockProperties;
      vector<string> m_expelledToCarrierBedPropertiesS;
      vector<string> m_expelledToSourceRockPropertiesS;

      bool computeSourceRock ( Genex6::SourceRock * aSourceRock, const Interface::Formation * aFormation );

   };

}

#endif 


