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

      /// \brief Get the undefined value.
      virtual double getUndefinedValue () const;

      /// \brief Return false.
      bool isPrimary () const;

      /// \ brief Get the gridMap
      const DataAccess::Interface::GridMap* getGridMap() const;

   private :

      /// \brief 2 dimensional array containing the surface property values.
      boost::multi_array<double,2> m_values;

   };

   typedef boost::shared_ptr<DerivedSurfaceProperty> DerivedSurfacePropertyPtr;

}


inline bool DerivedProperties::DerivedSurfaceProperty::isPrimary () const {

   return false;
}

inline const DataAccess::Interface::GridMap* DerivedProperties::DerivedSurfaceProperty::getGridMap() const {

   return 0;
}

#endif // DERIVED_PROPERTIES__DERIVED_SURFACE_PROPERTY_H
