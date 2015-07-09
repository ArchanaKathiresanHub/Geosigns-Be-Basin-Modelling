#ifndef DERIVED_PROPERTIES__FORMATION_MAP_PROPERTY_H
#define DERIVED_PROPERTIES__FORMATION_MAP_PROPERTY_H

#include <vector>

#include <boost/shared_ptr.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"
#include "AbstractGrid.h"

#include "AbstractPropertyValues.h"

namespace DerivedProperties {

   /// \brief Stores the values of the designated property for the formation.
   ///
   /// The values of this property must be continuous such as temperature and pressure.
   /// Properties such as porosity may not an object of this type to store their
   /// values.
   /// The indices will use global index numbering.
   class FormationMapProperty : public AbstractPropertyValues {

   public :

      FormationMapProperty ( const DataModel::AbstractProperty*  property,
                             const DataModel::AbstractSnapshot*  snapshot,
                             const DataModel::AbstractFormation* formation,
                             const DataModel::AbstractGrid*      grid );

      virtual ~FormationMapProperty () {}

      /// \brief Get the grid on which the property values are defined.
      const DataModel::AbstractGrid* getGrid () const;

      /// \brief Get the formation for which the property values are defined.
      const DataModel::AbstractFormation* getFormation () const;

      /// \brief Get the snapshot at which the property values are defined.
      const DataModel::AbstractSnapshot* getSnapshot () const;

      /// \brief Get the property.
      const DataModel::AbstractProperty* getProperty () const;


      /// \brief The first index on the grid in the x-direction.
      unsigned int firstI ( const bool includeGhostNodes ) const;

      /// \brief The first index on the grid in the y-direction.
      unsigned int firstJ ( const bool includeGhostNodes ) const;

      /// \brief The last index on the local grid in the x-direciton.
      unsigned int lastI ( const bool includeGhostNodes ) const;

      /// \brief The last index on the local grid in the y-direciton.
      unsigned int lastJ ( const bool includeGhostNodes ) const;


      /// \brief Get the value of the property at the position i,j.
      virtual double get ( unsigned int i,
                           unsigned int j ) const = 0;

      /// \brief Interpolate the value of the property at the position i,j.
      virtual double interpolate ( double i,
                                   double j ) const;

   private :

      const DataModel::AbstractProperty*  m_property;
      const DataModel::AbstractSnapshot*  m_snapshot;
      const DataModel::AbstractFormation* m_formation;
      const DataModel::AbstractGrid*      m_grid;

   };


   typedef boost::shared_ptr<const FormationMapProperty> FormationMapPropertyPtr;

   typedef std::vector<FormationMapPropertyPtr> FormationMapPropertyList;

} // namespace DerivedProperties

//--------------------------------
//  Inlined functions
//--------------------------------

inline const DataModel::AbstractGrid* DerivedProperties::FormationMapProperty::getGrid () const {
   return m_grid;
}

inline const DataModel::AbstractFormation* DerivedProperties::FormationMapProperty::getFormation () const {
   return m_formation;
}

inline const DataModel::AbstractSnapshot* DerivedProperties::FormationMapProperty::getSnapshot () const {
   return m_snapshot;
}

inline const DataModel::AbstractProperty* DerivedProperties::FormationMapProperty::getProperty () const {
   return m_property;
}

inline unsigned int DerivedProperties::FormationMapProperty::firstI ( const bool includeGhostNodes ) const {
   return static_cast<unsigned int>(m_grid->firstI ( includeGhostNodes ));
}

inline unsigned int DerivedProperties::FormationMapProperty::firstJ ( const bool includeGhostNodes ) const {
   return static_cast<unsigned int>(m_grid->firstJ ( includeGhostNodes ));
}

inline unsigned int DerivedProperties::FormationMapProperty::lastI ( const bool includeGhostNodes ) const {
   return static_cast<unsigned int>(m_grid->lastI ( includeGhostNodes ));
}

inline unsigned int DerivedProperties::FormationMapProperty::lastJ ( const bool includeGhostNodes ) const {
   return static_cast<unsigned int>(m_grid->lastJ ( includeGhostNodes ));
}

#endif // DERIVED_PROPERTIES__FORMATION_MAP_PROPERTY_H
