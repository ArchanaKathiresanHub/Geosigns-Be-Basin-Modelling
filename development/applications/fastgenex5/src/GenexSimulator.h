#ifndef _GENEXSIMULATION_GENEXSIMULATOR_H_
#define _GENEXSIMULATION_GENEXSIMULATOR_H_

#include <string>
#include <vector>
namespace database
{
   class Database;
}
#include "Interface/Interface.h"
#include "Interface/ProjectHandle.h"
#include "GenexSimulation.h"

#include "AdsorptionProjectHandle.h"

#include"petsc.h"

using namespace DataAccess;


namespace GenexSimulation
{
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
      bool run(void);

      void setAdsorptionSimulation ( AdsorptionProjectHandle* adsorptionProjectHandle );

      /// Save results
      bool saveTo (const std::string & outputFileName);

      /// Check if  propertyName is among the fastgenex5 requested properties
      bool isPropertyRequested (const string & propertyName) const;

      /// Determine whether or not the property-name passed is a shale-gas property.
      bool isShaleGasProperty ( const string & propertyName ) const;

      /// Read only access to the requested properties
      const std::vector<string> & getRequestedProperties() const;

      /// Read only access to the registered properties
      const std::vector<string> & getRegisteredProperties() const;

      /// Delete property values associated with generation and expulsion simulation
      void deleteSourceRockPropertyValues();

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
      vector<string> m_requestedProperties;

      /// All shale-gas property names.
      vector<string> m_shaleGasProperties;

      GenexSimulator &operator=(const GenexSimulator &);
      GenexSimulator (const GenexSimulator &);

      AdsorptionProjectHandle* m_adsorptionProjectHandle;

   };
}

#endif 


