//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__DERIVED_SURFACE_PROPERTY_H
#define DERIVED_PROPERTIES__DERIVED_SURFACE_PROPERTY_H

#include <boost/shared_ptr.hpp>
#include <boost/multi_array.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"
#include "AbstractGrid.h"

#include "SurfaceProperty.h"

namespace DerivedProperties {

   /// \brief Contains the values of the property at the snapshot time for the surface.
   ///
   /// The values are stored in a two dimensional array.
   class DerivedSurfaceProperty : public AbstractDerivedProperties::SurfaceProperty {

   public :

      DerivedSurfaceProperty ( const DataModel::AbstractProperty* property,
                               const DataModel::AbstractSnapshot* snapshot,
                               const DataModel::AbstractSurface*  surface,
                               const DataModel::AbstractGrid*     grid );

      virtual ~DerivedSurfaceProperty ();


      /// \brief Get the value at the position.
      virtual double get ( unsigned int i,
                           unsigned int j ) const;

      /// \brief Set a value at the position.
      virtual void set ( unsigned int i,
                         unsigned int j,
                         double   value );

      /// \brief Get the undefined value.
      virtual double getUndefinedValue () const;

      /// \brief Fill the property with the defined value
      void fill( const double value );

      /// \brief Return false.
      bool isPrimary () const;

   private :

      /// \brief 2 dimensional array containing the surface property values.
      boost::multi_array<double,2> m_values;

   };

   typedef boost::shared_ptr<DerivedSurfaceProperty> DerivedSurfacePropertyPtr;

}


inline bool DerivedProperties::DerivedSurfaceProperty::isPrimary () const {
   return false;
}

#endif // DERIVED_PROPERTIES__DERIVED_SURFACE_PROPERTY_H
