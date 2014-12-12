#ifndef _DATAACCESS__DOMAIN_PROPERTY_H_
#define _DATAACCESS__DOMAIN_PROPERTY_H_

#include "Interface/ProjectHandle.h"
#include "Interface/Snapshot.h"
#include "Interface/Property.h"

#include "ElementPosition.h"
#include "InterpolatedPropertyValues.h"

namespace DataAccess
{

   namespace Mining
   {
      /// Forward declaration of property collection.
      class DomainPropertyCollection;
 
      /// Objects of this type are capable of computing the value of a property at any point in the Cauldron domain.
      ///
      /// If the property is not valid at a particular point, e.g. the point does not lies on a surface 
      /// for a GenEx property then a null value will be returned.
      /// If the point lies outside of the valid Cauldron domain then a null value will be returned.
      class DomainProperty
      {
      public :

         DomainProperty( const DomainPropertyCollection * collection,
                         const Interface::Snapshot      * snapshot,
                         const Interface::Property      * property );

         virtual ~DomainProperty();

         /// Get the associated property.
         const Interface::Property * getProperty() const { return m_property; }

         /// Get the snapshot to which the results apply.
         const Interface::Snapshot * getSnapshot() const { return m_snapshot; }

         /// Get the project-handle.
         const Interface::ProjectHandle * getProjectHandle() const;

         /// Get the property-collection.
         const DomainPropertyCollection * getPropertyCollection() const { return m_collection; }

         /// Initialise the domain-property calculator.
         ///
         /// Return true is initialisation was okay, otherwise return false.
         /// The default behaviour is 'do nothing'.
         virtual bool initialise();

         /// Compute the value of the property at the position defined in element-position.
         ///
         /// Since the property may contain several sub-properties, e.g. FluidVelocity
         /// is made up from FluidVelocityX, FluidVelocityY and FluidVelocityZ, 
         /// the interpolated-values will be ...
         virtual void compute( const ElementPosition            & position,
                                     InterpolatedPropertyValues & evaluations ) const;

         /// Compute the value of the property at the position defined in element-position.
         ///
         /// Just a temporary function for debugging purposes.
         virtual double compute( const ElementPosition & position ) const = 0;

      private :
         const DomainPropertyCollection * m_collection; ///< The collection of all allocated properties 
         const Interface::Snapshot      * m_snapshot;   ///< The snapshot to which the result apply.
         const Interface::Property      * m_property;   ///< The property to which the results apply.

      };


      /// Functor class for allocating DomainProperty objects.
      class DomainPropertyAllocator
      {
      public :
         DomainPropertyAllocator() {;}
         virtual ~DomainPropertyAllocator() {;}

         /// Allocate a DomainProperty-derived object.
         virtual DomainProperty * allocate ( const DomainPropertyCollection *  collection,
                                             const Interface::Snapshot      * snapshot,
                                             const Interface::Property      * property ) const = 0;
      };
   }
}
#endif // _DATAACCESS__DOMAIN_PROPERTY_H_
