#ifndef DERIVED_PROPERTIES__PROPERTY_ERASE_PREDICATE_H
#define DERIVED_PROPERTIES__PROPERTY_ERASE_PREDICATE_H

#include "AbstractSnapshot.h"

namespace DerivedProperties {

   /// \brief To be used when removing properties form the list of existing properties.
   ///
   /// Returns true when the property contains the same snapshot and used when creating the predicate object.
   template<typename PropertyValuesPtr>
   class PropertyErasePredicate {

   public :

      /// \brief Constructor.
      ///
      /// \param [in] snapshot The snapshot at which all properties should be removed.
      PropertyErasePredicate ( const DataModel::AbstractSnapshot* snapshot );

      /// \brief Indicate whether or not the property contains the required snapshot.
      bool operator ()( const PropertyValuesPtr& prop ) const;

   private :

      /// \brief The snapshot at which all properties are to be removed.
      const DataModel::AbstractSnapshot* m_snapshot;

   };

} // namespace DerivedProperties

//--------------------------------
//  Inlined functions
//--------------------------------

template<typename PropertyValuesPtr>
inline DerivedProperties::PropertyErasePredicate<PropertyValuesPtr>::PropertyErasePredicate ( const DataModel::AbstractSnapshot* snapshot ) :
   m_snapshot ( snapshot )
{
}

template<typename PropertyValuesPtr>
inline bool DerivedProperties::PropertyErasePredicate<PropertyValuesPtr>::operator ()( const PropertyValuesPtr& prop ) const {
   return prop->getSnapshot () == m_snapshot;
}

#endif // DERIVED_PROPERTIES__PROPERTY_ERASE_PREDICATE_H
