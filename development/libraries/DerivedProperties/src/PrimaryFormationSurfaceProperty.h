#ifndef DERIVED_PROPERTIES__PRIMARY_FORMATION_SURFACE_PROPERTY_H
#define DERIVED_PROPERTIES__PRIMARY_FORMATION_SURFACE_PROPERTY_H

#include <boost/shared_ptr.hpp>
#include <boost/multi_array.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"
#include "AbstractGrid.h"


#include "Interface/GridMap.h"
#include "Interface/PropertyValue.h"

#include "FormationSurfaceProperty.h"

namespace DerivedProperties {

   /// \brief Contains the values of the property at the snapshot time for the surface.
   ///
   /// The values are stored in a two dimensional array.
   class PrimaryFormationSurfaceProperty : public FormationSurfaceProperty {


   public :

      PrimaryFormationSurfaceProperty ( const DataAccess::Interface::PropertyValue* propertyValue );

      ~PrimaryFormationSurfaceProperty();

      /// \brief Get the value at the position.
      virtual double get ( unsigned int i,
                           unsigned int j ) const;

      /// \brief Get the undefined value.
      virtual double getUndefinedValue () const;
   private :

      const DataAccess::Interface::PropertyValue* m_propertyValue;
      const DataAccess::Interface::GridMap* m_gridMap;

   };

   typedef boost::shared_ptr<PrimaryFormationSurfaceProperty> PrimaryFormationSurfacePropertyPtr;

}


#endif // DERIVED_PROPERTIES__PRIMARY_FORMATION_SURFACE_PROPERTY_H
