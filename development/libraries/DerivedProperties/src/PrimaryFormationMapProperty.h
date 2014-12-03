#ifndef DERIVED_PROPERTIES__PRIMARY_FORMATION_MAP_PROPERTY_H
#define DERIVED_PROPERTIES__PRIMARY_FORMATION_MAP_PROPERTY_H

#include <boost/shared_ptr.hpp>
#include <boost/multi_array.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"
#include "AbstractGrid.h"


#include "Interface/GridMap.h"
#include "Interface/PropertyValue.h"

#include "FormationMapProperty.h"

namespace DerivedProperties {

   /// \brief Contains the values of the property at the snapshot time for the surface.
   ///
   /// The values are stored in a two dimensional array.
   class PrimaryFormationMapProperty : public FormationMapProperty {


   public :

      PrimaryFormationMapProperty ( const DataAccess::Interface::PropertyValue* propertyValue );

      ~PrimaryFormationMapProperty();

      /// \brief Get the value at the position.
      virtual double get ( unsigned int i,
                           unsigned int j ) const;

      /// \brief Get the undefined value.
      virtual double getUndefinedValue () const;

      /// \brief Determine wether or not the data has been retrieved.
      virtual bool isRetrieved () const;

      /// \brief Retreive the grid map
      virtual void retrieveData () const;

      /// \brief Restore the grid map.
      virtual void restoreData () const;

   private :

      const DataAccess::Interface::PropertyValue* m_propertyValue;
      const DataAccess::Interface::GridMap* m_gridMap;

   };

   typedef boost::shared_ptr<PrimaryFormationMapProperty> PrimaryFormationMapPropertyPtr;

}


#endif // DERIVED_PROPERTIES__PRIMARY_FORMATION_MAP_PROPERTY_H

