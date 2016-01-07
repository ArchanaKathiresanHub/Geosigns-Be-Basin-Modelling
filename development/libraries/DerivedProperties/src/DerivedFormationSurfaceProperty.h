#ifndef _DERIVED_PROPERTIES__DERIVED_FORMATION_SURFACE_PROPERTY_H_
#define _DERIVED_PROPERTIES__DERIVED_FORMATION_SURFACE_PROPERTY_H_

#include <boost/multi_array.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"
#include "AbstractFormation.h"
#include "AbstractGrid.h"

#include "FormationSurfaceProperty.h"

namespace DerivedProperties {

   /// \brief Stores the values of the designated property for the formation.
   ///
   /// The indices will use global index numbering.
   class DerivedFormationSurfaceProperty : public FormationSurfaceProperty {

   public :

      DerivedFormationSurfaceProperty ( const DataModel::AbstractProperty*  property,
                                        const DataModel::AbstractSnapshot*  snapshot,
                                        const DataModel::AbstractFormation* formation,
                                        const DataModel::AbstractSurface*   surface,
                                        const DataModel::AbstractGrid*      grid );

      virtual ~DerivedFormationSurfaceProperty () {};

      /// \brief Get the value of the property at the position i,j.
      virtual double get ( unsigned int i,
                           unsigned int j ) const;

      /// \brief Set the value of the property at the position i,j.
      virtual void set ( unsigned int i,
                         unsigned int j,
                         double   value );

      /// \brief Get the undefined value.
      virtual double getUndefinedValue () const;

      /// \brief Return false.
      bool isPrimary () const;
   private :

      boost::multi_array<double,2> m_values;

   };

   typedef boost::shared_ptr<DerivedFormationSurfaceProperty> DerivedFormationSurfacePropertyPtr;

}

#endif // _DERIVED_PROPERTIES__DERIVED_FORMATION_SURFACE_PROPERTY_H_
