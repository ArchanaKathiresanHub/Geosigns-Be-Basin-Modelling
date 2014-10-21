#ifndef DERIVED_PROPERTIES__FORMATION_PROPERTY_ERASE_PREDICATE_H
#define DERIVED_PROPERTIES__FORMATION_PROPERTY_ERASE_PREDICATE_H

#include "AbstractSnapshot.h"
#include "FormationProperty.h"

namespace DerivedProperties {

   /// \brief To be used when removing formation properties form the list of existing formation properties.
   ///
   /// Returns true when the formation-property contains the same snapshot and used when creating the predicate object.
   class FormationPropertyErasePredicate {

   public :

      /// \brief Constructor.
      ///
      /// \param [in] snapshot The snapshot at which all formation-properties should be removed.
      FormationPropertyErasePredicate ( const DataModel::AbstractSnapshot* snapshot );

      /// \brief Indicate whether or not the formation-property contains the required snapshot.
      bool operator ()( const FormationPropertyPtr& prop ) const;

   private :

      /// \brief The snapshot at which all formation-properties are to be removed.
      const DataModel::AbstractSnapshot* m_snapshot;

   };

} // namespace DerivedProperties

//--------------------------------
//  Inlined functions
//--------------------------------

inline DerivedProperties::FormationPropertyErasePredicate::FormationPropertyErasePredicate ( const DataModel::AbstractSnapshot* snapshot ) :
   m_snapshot ( snapshot )
{
}

inline bool DerivedProperties::FormationPropertyErasePredicate::operator ()( const FormationPropertyPtr& prop ) const {
   return prop->getSnapshot () == m_snapshot;
}

#endif // DERIVED_PROPERTIES__FORMATION_PROPERTY_ERASE_PREDICATE_H
