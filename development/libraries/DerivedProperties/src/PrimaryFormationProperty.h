#ifndef DERIVED_PROPERTIES__PRIMARY_FORMATION_PROPERTY_H
#define DERIVED_PROPERTIES__PRIMARY_FORMATION_PROPERTY_H

#include <boost/shared_ptr.hpp>
#include <boost/multi_array.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"
#include "AbstractGrid.h"

#include "Interface/GridMap.h"
#include "Interface/PropertyValue.h"

#include "FormationProperty.h"

namespace DerivedProperties {

   /// \brief Contains the values of the property at the snapshot time for the formation.
   ///
   /// The values are stored in a two dimensional array.
   class PrimaryFormationProperty : public FormationProperty  {

   public :

      PrimaryFormationProperty ( const DataAccess::Interface::PropertyValue* propertyValue );

      ~PrimaryFormationProperty();

      /// \brief Get the value at the position.
      virtual double get ( unsigned int i,
                           unsigned int j,
                           unsigned int k ) const;

      /// \brief Get the undefined value.
      virtual double getUndefinedValue () const;
 private :
      const DataAccess::Interface::GridMap* m_gridMap;

   };

   typedef boost::shared_ptr<PrimaryFormationProperty> PrimaryFormationPropertyPtr;

}


#endif // DERIVED_PROPERTIES__PRIMARY_FORMATION_PROPERTY_H
