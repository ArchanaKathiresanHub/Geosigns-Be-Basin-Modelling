//                                                                      
// Copyright (C) 2017-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef DATA_MODEL__MOCK_COMPOUNDLITHOLOGY_H
#define DATA_MODEL__MOCK_COMPOUNDLITHOLOGY_H

#include <string>
#include <iostream>

#include "AbstractCompoundLithology.h"

namespace DataModel {

   /// \brief A CompoundLithology.
   class MockCompoundLithology final : public AbstractCompoundLithology {

   public :
      MockCompoundLithology ();

      virtual ~MockCompoundLithology() = default;

      /// @return the heat-production value for the compound-lithology.
      /// Mock class returns 1.55472 which gives 50 [API] GR value at 50% porosity!
      virtual double heatproduction() const final;

   private : 

       const double m_heatproduction;
   };

}

inline DataModel::MockCompoundLithology::MockCompoundLithology () : m_heatproduction(1.55472) {
  //set m_heatproduction to 1.55472, with a porosity of 50% the GR should be 50[API]. 
}



inline double DataModel::MockCompoundLithology::heatproduction() const {
   return m_heatproduction;
}


#endif // DATA_MODEL__MOCK_COMPOUNDLITHOLOGY_H
