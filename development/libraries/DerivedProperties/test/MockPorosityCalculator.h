//                                                                      
// Copyright (C) 2017-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef DERIVED_PROPERTIES__MOCKPOROSITYCALCULATOR_H
#define DERIVED_PROPERTIES__MOCKPOROSITYCALCULATOR_H

#include "FormationPropertyCalculator.h"
#include "AbstractPropertyManager.h"

namespace DataModel{
   
   /// \brief A Mock porosity calculator class
   ///
   /// This class is used for testing
   class MockPorosityCalculator final: public DerivedProperties::FormationPropertyCalculator {
   
   public :
   
      MockPorosityCalculator ();
   
      /// Calculate the mock prosoity of a formation
      /// 
      /// \param [in]  propertyManager The property manager.
      /// \param [in]  snapshot        The snapshot time for which the porosity is requested.
      /// \param [in]  formation       The formation for which the porosity is requested.
      /// \param [out] derivedProperties On exit will contain a single formation property, the porosity in the layer.
      /// \pre snapshot points to a valid snapshot age.
      /// \pre formation points to a valid formation.
      void calculate (       DerivedProperties::AbstractPropertyManager& propertyManager,
                       const AbstractSnapshot*                snapshot,
                       const AbstractFormation*               formation,
                             DerivedProperties::FormationPropertyList&   derivedProperties ) const final;
   
   private :
      
       static constexpr int numberOfGridNodes = 11;

   };
}
#endif // DERIVED_PROPERTIES__MOCKPOROSITYCALCULATOR_H
