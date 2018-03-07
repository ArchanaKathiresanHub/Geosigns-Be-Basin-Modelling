// 
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef ABSTRACTDERIVED_PROPERTIES__SURFACE_PROPERTY_H
#define ABSTRACTDERIVED_PROPERTIES__SURFACE_PROPERTY_H

#include <vector>

#include <boost/shared_ptr.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"
#include "AbstractGrid.h"

#include "AbstractPropertyValues.h"

namespace AbstractDerivedProperties {

   /// \brief Stores the values of the designated property for the surface.
   ///
   /// The values of this property must be continuous such as temperature and pressure.
   /// Properties such as porosity may not an object of this type to store their
   /// values.
   /// The indices will use global index numbering.
   class SurfaceProperty : public AbstractPropertyValues {

   public :

      SurfaceProperty ( const DataModel::AbstractProperty* property,
                        const DataModel::AbstractSnapshot* snapshot,
                        const DataModel::AbstractSurface*  surface,
                        const DataModel::AbstractGrid*     grid );

      virtual ~SurfaceProperty () = default;

      /// \brief Get the grid on which the property values are defined.
      const DataModel::AbstractGrid* getGrid () const;

      /// \brief Get the surface for which the property values are defined.
      const DataModel::AbstractSurface* getSurface () const;

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

      const DataModel::AbstractProperty* m_property;
      const DataModel::AbstractSnapshot* m_snapshot;
      const DataModel::AbstractSurface*  m_surface;
      const DataModel::AbstractGrid*     m_grid;

   };


   typedef boost::shared_ptr<const SurfaceProperty> SurfacePropertyPtr;

   typedef std::vector<SurfacePropertyPtr> SurfacePropertyList;

} // namespace AbstractDerivedProperties

//--------------------------------
//  Inlined functions
//--------------------------------

inline const DataModel::AbstractGrid* AbstractDerivedProperties::SurfaceProperty::getGrid () const {
   return m_grid;
}

inline const DataModel::AbstractSurface* AbstractDerivedProperties::SurfaceProperty::getSurface () const {
   return m_surface;
}

inline const DataModel::AbstractSnapshot* AbstractDerivedProperties::SurfaceProperty::getSnapshot () const {
   return m_snapshot;
}

inline const DataModel::AbstractProperty* AbstractDerivedProperties::SurfaceProperty::getProperty () const {
   return m_property;
}

inline unsigned int AbstractDerivedProperties::SurfaceProperty::firstI ( const bool includeGhostNodes ) const {
   return static_cast<unsigned int>(m_grid->firstI ( includeGhostNodes ));
}

inline unsigned int AbstractDerivedProperties::SurfaceProperty::firstJ ( const bool includeGhostNodes ) const {
   return static_cast<unsigned int>(m_grid->firstJ ( includeGhostNodes ));
}

inline unsigned int AbstractDerivedProperties::SurfaceProperty::lastI ( const bool includeGhostNodes ) const {
   return static_cast<unsigned int>(m_grid->lastI ( includeGhostNodes ));
}

inline unsigned int AbstractDerivedProperties::SurfaceProperty::lastJ ( const bool includeGhostNodes ) const {
   return static_cast<unsigned int>(m_grid->lastJ ( includeGhostNodes ));
}


#endif // ABSTRACTDERIVED_PROPERTIES__SURFACE_PROPERTY_H
