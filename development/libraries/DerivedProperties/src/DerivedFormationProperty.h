#ifndef _DERIVED_PROPERTIES__DERIVED_FORMATION_PROPERTY_H_
#define _DERIVED_PROPERTIES__DERIVED_FORMATION_PROPERTY_H_

#include <boost/multi_array.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"
#include "AbstractGrid.h"

#include "FormationProperty.h"

namespace DerivedProperties {

   /// \brief Stores the values of the designated property for the formation.
   ///
   /// The indices will use global index numbering.
   class DerivedFormationProperty : public FormationProperty {

   public :

      DerivedFormationProperty ( const DataModel::AbstractProperty*  property,
                                 const DataModel::AbstractSnapshot*  snapshot,
                                 const DataModel::AbstractFormation* formation,
                                 const DataModel::AbstractGrid*      grid,
                                 const unsigned int                  nk );


      /// \brief Get the value of the property at the position i,j,k.
      virtual double get ( unsigned int i,
                           unsigned int j,
                           unsigned int k ) const;

      /// \brief Get the value of the property at the position i,j,k.
      void set ( unsigned int i,
                 unsigned int j,
                 unsigned int k,
                 double   value );


   private :

      /// \brief 3D array containing the property values for the formation.
      ///
      /// There will be one value for each node in the formation.
      boost::multi_array<double,3> m_values;

   };

} // namespace DerivedProperties

//--------------------------------
//  Inlined functions
//--------------------------------

inline double DerivedProperties::DerivedFormationProperty::get ( unsigned int i,
                                                                 unsigned int j,
                                                                 unsigned int k ) const {
   return m_values [ i - firstI(true)][ j - firstJ(true)][ k - firstK ()];
}

#endif // _DERIVED_PROPERTIES__DERIVED_FORMATION_PROPERTY_H_
