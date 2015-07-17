#ifndef DERIVED_PROPERTIES__DERIVED_FORMATION_MAP_PROPERTY_H
#define DERIVED_PROPERTIES__DERIVED_FORMATION_MAP_PROPERTY_H

#include <boost/shared_ptr.hpp>
#include <boost/multi_array.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"
#include "AbstractGrid.h"

#include "FormationMapProperty.h"

namespace DerivedProperties {

   /// \brief Contains the values of the property at the snapshot time for the formation.
   ///
   /// The values are stored in a two dimensional array.
   class DerivedFormationMapProperty : public FormationMapProperty {

   public :

      DerivedFormationMapProperty ( const DataModel::AbstractProperty*  property,
                                    const DataModel::AbstractSnapshot*  snapshot,
                                    const DataModel::AbstractFormation* formation,
                                    const DataModel::AbstractGrid*      grid );

      virtual ~DerivedFormationMapProperty () {};


      /// \brief Get the value at the position.
      virtual double get ( unsigned int i,
                           unsigned int j ) const;

      /// \brief Set a value at the position.
      virtual void set ( unsigned int i,
                         unsigned int j,
                         double   value );

      /// \brief Get the undefined value.
      virtual double getUndefinedValue () const;

   private :

      /// \brief 2 dimensional array containing the formation map property values.
      boost::multi_array<double,2> m_values;

   };

   typedef boost::shared_ptr<DerivedFormationMapProperty> DerivedFormationMapPropertyPtr;

}


#endif // DERIVED_PROPERTIES__DERIVED_FORMATION_MAP_PROPERTY_H
