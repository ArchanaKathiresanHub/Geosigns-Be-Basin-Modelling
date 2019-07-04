//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _ABSTRACTDERIVED_PROPERTIES__FORMATION_SURFACE_PROPERTY_H_
#define _ABSTRACTDERIVED_PROPERTIES__FORMATION_SURFACE_PROPERTY_H_

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"
#include "AbstractFormation.h"
#include "AbstractGrid.h"

#include "AbstractPropertyValues.h"

#include <memory>
#include <vector>

namespace AbstractDerivedProperties {

   /// \brief Stores the values of the designated property for the formation.
   ///
   /// The indices will use global index numbering.
   class FormationSurfaceProperty : public AbstractPropertyValues {

   public :

      FormationSurfaceProperty ( const DataModel::AbstractProperty*  property,
                                 const DataModel::AbstractSnapshot*  snapshot,
                                 const DataModel::AbstractFormation* formation,
                                 const DataModel::AbstractSurface*   surface,
                                 const DataModel::AbstractGrid*      grid );

      virtual ~FormationSurfaceProperty () {}

      /// \brief Get the grid on which the property values are defined.
      const DataModel::AbstractGrid* getGrid () const;

      /// \brief Get the formation for which the property values are defined.
      const DataModel::AbstractFormation* getFormation () const;

      /// \brief Get the surface for which the property values are defined.
      const DataModel::AbstractSurface* getSurface () const;

      /// \brief Get the snapshot at which the property values are defined.
      const DataModel::AbstractSnapshot* getSnapshot () const;

      /// \brief Get the property.
      const DataModel::AbstractProperty* getProperty () const;


      /// \brief The first index on the grid
      unsigned int firstI ( const bool includeGhostNodes ) const;

      /// \brief The last index on the local grid.
      unsigned int lastI ( const bool includeGhostNodes ) const;

      unsigned int firstJ ( const bool includeGhostNodes ) const;

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
      const DataModel::AbstractSurface*   m_surface;
      const DataModel::AbstractFormation* m_formation;
      const DataModel::AbstractGrid*      m_grid;

   };


   typedef std::shared_ptr<const FormationSurfaceProperty> FormationSurfacePropertyPtr;

   typedef std::vector<FormationSurfacePropertyPtr> FormationSurfacePropertyList;

} // namespace AbstractDerivedProperties

//--------------------------------
//  Inlined functions
//--------------------------------

inline const DataModel::AbstractGrid* AbstractDerivedProperties::FormationSurfaceProperty::getGrid () const {
   return m_grid;
}

inline const DataModel::AbstractFormation* AbstractDerivedProperties::FormationSurfaceProperty::getFormation () const {
   return m_formation;
}

inline const DataModel::AbstractSurface* AbstractDerivedProperties::FormationSurfaceProperty::getSurface () const {
   return m_surface;
}

inline const DataModel::AbstractSnapshot* AbstractDerivedProperties::FormationSurfaceProperty::getSnapshot () const {
   return m_snapshot;
}

inline const DataModel::AbstractProperty* AbstractDerivedProperties::FormationSurfaceProperty::getProperty () const {
   return m_property;
}

inline unsigned int AbstractDerivedProperties::FormationSurfaceProperty::firstI ( const bool includeGhostNodes ) const {
   return static_cast<unsigned int>(m_grid->firstI ( includeGhostNodes ));
}

inline unsigned int AbstractDerivedProperties::FormationSurfaceProperty::lastI ( const bool includeGhostNodes ) const {
   return static_cast<unsigned int>(m_grid->lastI ( includeGhostNodes ));
}

inline unsigned int AbstractDerivedProperties::FormationSurfaceProperty::firstJ ( const bool includeGhostNodes ) const {
   return static_cast<unsigned int>(m_grid->firstJ ( includeGhostNodes ));
}

inline unsigned int AbstractDerivedProperties::FormationSurfaceProperty::lastJ ( const bool includeGhostNodes ) const {
   return static_cast<unsigned int>(m_grid->lastJ ( includeGhostNodes ));
}

#endif // _ABSTRACTDERIVED_PROPERTIES__FORMATION_SURFACE_PROPERTY_H_
