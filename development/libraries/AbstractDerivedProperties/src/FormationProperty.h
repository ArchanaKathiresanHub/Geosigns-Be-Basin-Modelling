//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _ABSTRACTDERIVED_PROPERTIES__FORMATION_PROPERTY_H_
#define _ABSTRACTDERIVED_PROPERTIES__FORMATION_PROPERTY_H_

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"
#include "AbstractGrid.h"

#include "NumericFunctions.h"

#include "AbstractPropertyValues.h"

#include <memory>
#include <vector>

namespace AbstractDerivedProperties {

   /// \brief Stores the values of the designated property for the formation.
   ///
   /// The indices will use global index numbering.
   /// The array bounds are a closed set of intervals.
   class FormationProperty : public AbstractPropertyValues {

   public :

      FormationProperty ( const DataModel::AbstractProperty*  property,
                          const DataModel::AbstractSnapshot*  snapshot,
                          const DataModel::AbstractFormation* formation,
                          const DataModel::AbstractGrid*      grid,
                          const unsigned int                  nk );

      virtual ~FormationProperty () = default;

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

      /// \brief The first index on the grid in the z-direction.
      unsigned int firstK () const;

      /// \brief The last index on the grid in the x-direction.
      ///
      /// This is the last index of a closed interval.
      unsigned int lastI ( const bool includeGhostNodes ) const;

      /// \brief The last index on the grid in the y-direction.
      ///
      /// This is the last index of a closed interval.
      unsigned int lastJ ( const bool includeGhostNodes ) const;

      /// \brief The last index on the grid in the z-direction.
      ///
      /// This is the last index of a closed interval.
      unsigned int lastK () const;

      /// \brief The number of nodes in the z-direction.
      unsigned int lengthK () const;

      /// \brief Get the value of the property at the position i,j,k.
      ///
      /// If any of the i, j or k values is out of range then a null value will be returned.
      virtual double checkedGet ( unsigned int i,
                                  unsigned int j,
                                  unsigned int k ) const;

      /// \brief Get the value of the property at the position i,j,k (ascending order)
      virtual double get ( unsigned int i,
                           unsigned int j,
                           unsigned int k ) const = 0;

      /// \brief Get the value of the property at the position i,j and interpolated at a position k.
      virtual double interpolate ( unsigned int i,
                                   unsigned int j,
                                   double       k ) const;

      /// \brief Get the value of the property at the position i,j and interpolated at a position k.
      virtual double interpolate ( double i,
                                   double j,
                                   double k ) const;

      /// \brief Return true if the property is primary.
      virtual bool isPrimary () const = 0;

   private :

      /// \brief A pointer to the property.
      const DataModel::AbstractProperty*  m_property;

      /// \brief A pointer to the snapshot.
      const DataModel::AbstractSnapshot*  m_snapshot;

      /// \brief A pointer to the formation.
      const DataModel::AbstractFormation* m_formation;

      /// \brief A pointer to the grid.
      const DataModel::AbstractGrid*      m_grid;

      /// \brief The nuber of nodes in the z-direction.
      const unsigned int                  m_numberK;

   };


   typedef std::shared_ptr<const FormationProperty> FormationPropertyPtr;

   typedef std::vector<FormationPropertyPtr> FormationPropertyList;

} // namespace AbstractDerivedProperties

//--------------------------------
//  Inlined functions
//--------------------------------

inline const DataModel::AbstractGrid* AbstractDerivedProperties::FormationProperty::getGrid () const {
   return m_grid;
}

inline const DataModel::AbstractFormation* AbstractDerivedProperties::FormationProperty::getFormation () const {
   return m_formation;
}

inline const DataModel::AbstractSnapshot* AbstractDerivedProperties::FormationProperty::getSnapshot () const {
   return m_snapshot;
}

inline const DataModel::AbstractProperty* AbstractDerivedProperties::FormationProperty::getProperty () const {
   return m_property;
}

inline unsigned int AbstractDerivedProperties::FormationProperty::firstI ( const bool includeGhostNodes ) const {
   return static_cast<unsigned int>(m_grid->firstI ( includeGhostNodes ));
}

inline unsigned int AbstractDerivedProperties::FormationProperty::lastI ( const bool includeGhostNodes ) const {
   return static_cast<unsigned int>(m_grid->lastI ( includeGhostNodes ));
}

inline unsigned int AbstractDerivedProperties::FormationProperty::firstJ ( const bool includeGhostNodes ) const {
   return static_cast<unsigned int>(m_grid->firstJ ( includeGhostNodes ));
}

inline unsigned int AbstractDerivedProperties::FormationProperty::lastJ ( const bool includeGhostNodes ) const {
   return static_cast<unsigned int>(m_grid->lastJ ( includeGhostNodes ));
}

inline unsigned int AbstractDerivedProperties::FormationProperty::firstK () const {
   return 0;
}

inline unsigned int AbstractDerivedProperties::FormationProperty::lastK () const {
   return m_numberK - 1;
}

inline unsigned int AbstractDerivedProperties::FormationProperty::lengthK () const {
   return m_numberK;
}

inline double AbstractDerivedProperties::FormationProperty::checkedGet ( unsigned int i,
                                                                         unsigned int j,
                                                                         unsigned int k ) const {

   if ( NumericFunctions::inRange<unsigned int>( i, firstI ( true ), lastI ( true )) and
        NumericFunctions::inRange<unsigned int>( j, firstJ ( true ), lastJ ( true )) and
        NumericFunctions::inRange<unsigned int>( k, firstK (), lastK ())) {

         return get ( i, j, k );
   } else {
      return getUndefinedValue ();
   }
}

#endif // _ABSTRACTDERIVED_PROPERTIES__FORMATION_PROPERTY_H_
