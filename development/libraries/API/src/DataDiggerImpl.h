//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file DataDiggerImpl.h
/// @brief This file keeps API implementation declaration for data digger. 

#ifndef CASA_API_DATA_DIGGER_IMPL_H
#define CASA_API_DATA_DIGGER_IMPL_H

#include "DataDigger.h"


namespace casa
{  
   // Data digger implementation 
   class DataDiggerImpl : public DataDigger
   {
   public:
      /// Constructor / Destructor
      DataDiggerImpl();
      virtual ~DataDiggerImpl();

      // Collect observables value from simulation results for given case
      // cs RunCase object which keeps list of observables and reference to Cauldron model
      // return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode collectRunResults( RunCase & cs );

   protected:
   };
}

#endif // CASA_API_DATA_DIGGER_IMPL_H
