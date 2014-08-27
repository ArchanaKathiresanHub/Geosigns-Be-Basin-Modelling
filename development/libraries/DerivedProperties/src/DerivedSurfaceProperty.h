#ifndef _DERIVED_PROPERTIES__DERIVED_SURFACE_PROPERTY_H_
#define _DERIVED_PROPERTIES__DERIVED_SURFACE_PROPERTY_H_

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
   class DerivedSurfaceProperty : public SurfaceProperty {

   public :

      DerivedSurfaceProperty ( const DataModel::AbstractProperty* property,
                               const DataModel::AbstractSnapshot* snapshot,
                               const DataModel::AbstractSurface*  surface,
                               const DataModel::AbstractGrid*     grid );

      virtual ~DerivedSurfaceProperty () {};


      /// \brief Get the value at the position.
      virtual double get ( unsigned int i,
                           unsigned int j ) const;

      /// \brief Set a value at the position.
      virtual void set ( unsigned int i,
                         unsigned int j,
                         double   value );

   private :

      boost::multi_array<double,2> m_values;

   };

}


#endif // _DERIVED_PROPERTIES__DERIVED_SURFACE_PROPERTY_H_
