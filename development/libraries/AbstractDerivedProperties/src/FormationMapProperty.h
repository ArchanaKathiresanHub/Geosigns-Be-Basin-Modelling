//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef ABSTRACTDERIVED_PROPERTIES__FORMATION_MAP_PROPERTY_H
#define ABSTRACTDERIVED_PROPERTIES__FORMATION_MAP_PROPERTY_H

#include <vector>

#include <boost/shared_ptr.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"
#include "AbstractGrid.h"

#include "AbstractPropertyValues.h"

namespace AbstractDerivedProperties {

   /// \brief Stores the values of the designated property for the formation.
   ///
   /// The indices will use global index numbering.
   class FormationMapProperty : public AbstractPropertyValues {

   public :

      FormationMapProperty ( const DataModel::AbstractProperty*  property,
                             const DataModel::AbstractSnapshot*  snapshot,
                             const DataModel::AbstractFormation* formation,
                             const DataModel::AbstractGrid*      grid );

      virtual ~FormationMapProperty () = default;

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

      /// \brief Return true if the property is primary.
      virtual bool isPrimary () const = 0;

   private :

      const DataModel::AbstractProperty*  m_property;
      const DataModel::AbstractSnapshot*  m_snapshot;
      const DataModel::AbstractFormation* m_formation;
      const DataModel::AbstractGrid*      m_grid;

   };


   typedef boost::shared_ptr<const FormationMapProperty> FormationMapPropertyPtr;

   typedef std::vector<FormationMapPropertyPtr> FormationMapPropertyList;

} // namespace AbstractDerivedProperties

//--------------------------------
//  Inlined functions
//--------------------------------

inline const DataModel::AbstractGrid* AbstractDerivedProperties::FormationMapProperty::getGrid () const {
   return m_grid;
}

inline const DataModel::AbstractFormation* AbstractDerivedProperties::FormationMapProperty::getFormation () const {
   return m_formation;
}

inline const DataModel::AbstractSnapshot* AbstractDerivedProperties::FormationMapProperty::getSnapshot () const {
   return m_snapshot;
}

inline const DataModel::AbstractProperty* AbstractDerivedProperties::FormationMapProperty::getProperty () const {
   return m_property;
}

inline unsigned int AbstractDerivedProperties::FormationMapProperty::firstI ( const bool includeGhostNodes ) const {
   return static_cast<unsigned int>(m_grid->firstI ( includeGhostNodes ));
}

inline unsigned int AbstractDerivedProperties::FormationMapProperty::firstJ ( const bool includeGhostNodes ) const {
   return static_cast<unsigned int>(m_grid->firstJ ( includeGhostNodes ));
}

inline unsigned int AbstractDerivedProperties::FormationMapProperty::lastI ( const bool includeGhostNodes ) const {
   return static_cast<unsigned int>(m_grid->lastI ( includeGhostNodes ));
}

inline unsigned int AbstractDerivedProperties::FormationMapProperty::lastJ ( const bool includeGhostNodes ) const {
   return static_cast<unsigned int>(m_grid->lastJ ( includeGhostNodes ));
}

#endif // ABSTRACTDERIVED_PROPERTIES__FORMATION_MAP_PROPERTY_H
