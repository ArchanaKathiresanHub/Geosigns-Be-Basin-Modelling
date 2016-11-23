#ifndef _GEOPHYSICS__LITHOLOGY_MANAGER_H_
#define _GEOPHYSICS__LITHOLOGY_MANAGER_H_

#include <map>
#include <vector>
#include <string>

#include "Interface/auxiliaryfaulttypes.h"

#include "SimpleLithology.h"
#include "CompoundLithology.h"
#include "CompoundLithologyComposition.h"

#include "GeoPhysicsProjectHandle.h"


namespace GeoPhysics {

   /// Holds all the simple and compound lithologies.
   /// Stores and creates, if necessary, the fault lithologies.
   class LithologyManager {


      typedef SimpleLithology* SimpleLithologyPtr;

      /// Change to map (lithoname --> simpleLithology)
      typedef std::vector<SimpleLithologyPtr> SimpleLithoList;

      typedef std::map< CompoundLithologyComposition, CompoundLithology*, less<CompoundLithologyComposition> > CompoundLithologyMapping;


   public :

      typedef CompoundLithologyMapping::const_iterator CompoundLithologyIterator;


      LithologyManager ( ProjectHandle* projectHandle );
      virtual ~LithologyManager ();

      /// Find a compound lithology based on the composition.
      /// If the composition does not exist then this will return null.
      CompoundLithology*  getCompoundLithology ( const CompoundLithologyComposition& composition );

      /// Find, or create, a compound lithology made up of just the single simple 
      /// lithology given by the simpleLithologyName.
      CompoundLithology*  getCompoundLithology ( const std::string& simpleLithologyName );

      int numberOfSimpleLithologies () const;

      void addSimpleLithology ( const SimpleLithologyPtr newSimpleLithology );

      /// Find a simple lithology based on the lithology name.
      /// If the name does not exist then this will return null.
      SimpleLithology* getSimpleLithology   ( const std::string& simpleLithologyName ) const;

      void printSimpleLithologies () const;

      /// Get the compound lithology based on the composition and the fault status.
      /// If the lithology does not exist then it will be created.
      CompoundLithology* getCompoundFaultLithology ( const CompoundLithologyComposition&              composition,
                                                     const DataAccess::Interface::PressureFaultStatus verticalStatus,
                                                     const DataAccess::Interface::PressureFaultStatus lateralStatus );

      /// Get the compound lithology based on the simple lithology name.
      /// If the lithology does not exist then it will be created.
      CompoundLithology*  getCompoundFaultLithology ( const std::string&                  simpleLithologyName,
                                                      const CompoundLithologyComposition& composition );


      // Need to sort these out!
      size_t size () const {
         return compoundLithologies.size ();
      }

      CompoundLithologyIterator begin () const;

      CompoundLithologyIterator end   () const;

   private :


      /// Get the simple lithology based on the fault name and the fault status.
      /// If it does not exist then it will be created. The simple lithology wil exist
      /// at this point.
      SimpleLithology* getSimpleFaultLithology ( const std::string&                                    simpleLithologyName,
                                                 const DataAccess::Interface::PressureFaultStatus verticalStatus,
                                                 const DataAccess::Interface::PressureFaultStatus lateralStatus );

      /// Generates a simple lithology name.
      std::string generateSimpleFaultLithologyName ( const std::string&                                    simpleLithologyName,
                                                     const DataAccess::Interface::PressureFaultStatus verticalStatus,
                                                     const DataAccess::Interface::PressureFaultStatus lateralStatus ) const;

      std::string generateSimpleFaultLithologyName ( const std::string&        simpleLithologyName ) const;

      /// Generates the composition for the fault lithology based on the compound lithology composition AND
      /// the fault status.
      void generateCompoundFaultLithologyComposition ( const CompoundLithologyComposition&              composition,
                                                       const DataAccess::Interface::PressureFaultStatus verticalStatus,
                                                       const DataAccess::Interface::PressureFaultStatus lateralStatus,
                                                             CompoundLithologyComposition&              faultComposition ) const;

      //    SimpleLithologyMapping   simpleLithologies;
      SimpleLithoList          simpleLithologies;
      CompoundLithologyMapping compoundLithologies;

      GeoPhysics::ProjectHandle* m_projectHandle;

   };

}

#endif // _GEOPHYSICS__LITHOLOGY_MANAGER_H_
