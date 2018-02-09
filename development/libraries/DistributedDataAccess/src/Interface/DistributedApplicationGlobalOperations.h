//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _DATA_ACCESS__DISTRIBUTED_APPLICATION_GLOBAL_OPERATIONS__H_
#define _DATA_ACCESS__DISTRIBUTED_APPLICATION_GLOBAL_OPERATIONS__H_

#include "Interface/ApplicationGlobalOperations.h"

namespace DataAccess {

   namespace Interface {

      /// \brief Class for handling global operations on distributed architectures.
      class DistributedApplicationGlobalOperations : public ApplicationGlobalOperations {

      public :

         ~DistributedApplicationGlobalOperations () = default;

         /// \brief Return the global maximum of the value.
         double maximum ( const double val ) const final;

         /// \brief Return the global minimum of the value.
         double minimum ( const double val ) const final;

         /// \brief Return the global sum of the values.
         double sum ( const double val ) const final;

      }; 

   }

}


#endif // _DATA_ACCESS__DISTRIBUTED_APPLICATION_GLOBAL_OPERATIONS__H_
