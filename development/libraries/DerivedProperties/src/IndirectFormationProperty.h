//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _DERIVED_PROPERTIES__INDIRECT_FORMATION_PROPERTY_H_
#define _DERIVED_PROPERTIES__INDIRECT_FORMATION_PROPERTY_H_

#include "FormationProperty.h"

namespace DerivedProperties
{

   class IndirectFormationProperty : public AbstractDerivedProperties::FormationProperty
   {

   public:

      /// \brief Input constructor
      /// \param [in]  i_property         Manager for all derived properties
      /// \param [in]  i_propertyValues   The snapshot at which the property is to be calculated
      IndirectFormationProperty( const DataModel::AbstractProperty*                    property,
                                       AbstractDerivedProperties::FormationPropertyPtr propertyValues );

      /// \brief Destructor
      virtual ~IndirectFormationProperty();

      /// \brief Get the value of the property at the position i,j,k (ascending order)
      virtual double get ( unsigned int i,
                           unsigned int j,
                           unsigned int k ) const;

      /// \brief Get the undefined value.
      virtual double getUndefinedValue () const;

      /// \brief Return false.
      bool isPrimary () const;

   private:

      AbstractDerivedProperties::FormationPropertyPtr m_property;    //!< Pointer to FormationProperty storing values
   };

   typedef boost::shared_ptr<IndirectFormationProperty> IndirectFormationPropertyPtr;

} // namespace DerivedProperties

//--------------------------------
//  Inlined functions
//--------------------------------

inline double DerivedProperties::IndirectFormationProperty::get( unsigned int i,
                                                                 unsigned int j,
                                                                 unsigned int k) const
{
   return m_property->get(i, j, k);
}

inline bool DerivedProperties::IndirectFormationProperty::isPrimary () const {

   return false; 
}

#endif // _DERIVED_PROPERTIES__INDIRECT_FORMATION_PROPERTY_H_
