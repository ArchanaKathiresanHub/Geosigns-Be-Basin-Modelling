//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__DERIVED_FORMATION_MAP_PROPERTY_H
#define DERIVED_PROPERTIES__DERIVED_FORMATION_MAP_PROPERTY_H

#include <boost/shared_ptr.hpp>
#include <boost/multi_array.hpp>

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"
#include "AbstractGrid.h"

#include "FormationMapProperty.h"

namespace DerivedProperties {

   /// \brief Contains the values of the property at the snapshot time for the formation.
   ///
   /// The values are stored in a two dimensional array.
   class DerivedFormationMapProperty : public AbstractDerivedProperties::FormationMapProperty {

   public :

      DerivedFormationMapProperty ( const DataModel::AbstractProperty*  property,
                                    const DataModel::AbstractSnapshot*  snapshot,
                                    const DataModel::AbstractFormation* formation,
                                    const DataModel::AbstractGrid*      grid );

      virtual ~DerivedFormationMapProperty ();


      /// \brief Get the value at the position.
      virtual double get ( unsigned int i,
                           unsigned int j ) const;

      /// \brief Set a value at the position.
      virtual void set ( unsigned int i,
                         unsigned int j,
                         double   value );

      /// \brief Get the undefined value.
      virtual double getUndefinedValue () const;

       /// \brief Return false
      bool isPrimary () const;

  private :

      /// \brief 2 dimensional array containing the formation map property values.
      boost::multi_array<double,2> m_values;

   };

   typedef boost::shared_ptr<DerivedFormationMapProperty> DerivedFormationMapPropertyPtr;

}

inline bool DerivedProperties::DerivedFormationMapProperty::isPrimary () const {

   return false;
}

#endif // DERIVED_PROPERTIES__DERIVED_FORMATION_MAP_PROPERTY_H
