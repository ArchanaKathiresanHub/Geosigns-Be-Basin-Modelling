//                                                                      
// Copyright (C) 2017-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef DATA_MODEL__ABSTRACT_COMPOUNDLITHOLOGY_H
#define DATA_MODEL__ABSTRACT_COMPOUNDLITHOLOGY_H

#include <string>
#include <iostream>


namespace DataModel {

   class AbstractCompoundLithology {

   public:

      virtual ~AbstractCompoundLithology () = default;

      /// @return the heat-production value for the compound-lithology.
      virtual double heatproduction() const = 0;
 
   };

} // namespace DataModel

#endif // DATA_MODEL__ABSTRACT_COMPOUNDLITHOLOGY_H
