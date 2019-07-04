//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _DERIVED_PROPERTIES__DERIVED_FORMATION_PROPERTY_H_
#define _DERIVED_PROPERTIES__DERIVED_FORMATION_PROPERTY_H_

#include <boost/multi_array.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"
#include "AbstractGrid.h"

#include "FormationProperty.h"

#include "GridMap.h"

#include <memory>

namespace DerivedProperties {

   /// \brief Stores the values of the designated property for the formation.
   ///
   /// The indices will use global index numbering.
   class DerivedFormationProperty : public AbstractDerivedProperties::FormationProperty {

   public :

      DerivedFormationProperty ( const DataModel::AbstractProperty*  property,
                                 const DataModel::AbstractSnapshot*  snapshot,
                                 const DataModel::AbstractFormation* formation,
                                 const DataModel::AbstractGrid*      grid,
                                 const unsigned int                  nk );

      virtual ~DerivedFormationProperty ();

      /// \brief Get the value of the property at the position i,j,k (ascending order)
      virtual double get ( unsigned int i,
                           unsigned int j,
                           unsigned int k ) const;

      /// \brief Get the value of the property at the position i,j,k.
      void set ( unsigned int i,
                 unsigned int j,
                 unsigned int k,
                 double   value );

      /// \brief Get the undefined value.
      virtual double getUndefinedValue () const;

      /// \brief Return false.
      bool isPrimary () const;

   private :

      /// \brief 3D array containing the property values for the formation.
      ///
      /// There will be one value for each node in the formation.
      boost::multi_array<double,3> m_values;

   };

   typedef std::shared_ptr<DerivedFormationProperty> DerivedFormationPropertyPtr;

} // namespace DerivedProperties

//--------------------------------
//  Inlined functions
//--------------------------------

inline double DerivedProperties::DerivedFormationProperty::get ( unsigned int i,
                                                                 unsigned int j,
                                                                 unsigned int k ) const {
   return m_values [ i - firstI(true)][ j - firstJ(true)][ k - firstK ()];
}

inline bool DerivedProperties::DerivedFormationProperty::isPrimary () const {

   return false;
}

#endif // _DERIVED_PROPERTIES__DERIVED_FORMATION_PROPERTY_H_
