#ifndef _DERIVED_PROPERTIES__DERIVED_SURFACE_POROSITY_H_
#define _DERIVED_PROPERTIES__DERIVED_SURFACE_POROSITY_H_

#include <boost/multi_array.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"
#include "AbstractGrid.h"

#include "SurfaceProperty.h"

namespace DerivedProperties {

   class DerivedSurfaceProperty : public SurfaceProperty {

   public :

      DerivedSurfaceProperty ( const DataModel::AbstractProperty* prop,
                               const DataModel::AbstractSnapshot* ss,
                               const DataModel::AbstractSurface*  surf,
                               const DataModel::AbstractGrid*     g );

      virtual ~DerivedSurfaceProperty () {};


      virtual double get ( unsigned int i,
                           unsigned int j ) const;

      virtual void set ( unsigned int i,
                         unsigned int j,
                         double   value );

   private :

      boost::multi_array<double,2> m_values;

   };

}


#endif // _DERIVED_PROPERTIES__DERIVED_SURFACE_POROSITY_H_
