#ifndef _DATA_ACCESS__DISTRIBUTED_APPLICATION_GLOBAL_OPERATIONS__H_
#define _DATA_ACCESS__DISTRIBUTED_APPLICATION_GLOBAL_OPERATIONS__H_

#include "Interface/ApplicationGlobalOperations.h"

namespace DataAccess {

   namespace Interface {

      /// \brief Class for handling global operations on distributed architectures.
      class DistributedApplicationGlobalOperations : public ApplicationGlobalOperations {

      public :

         ~DistributedApplicationGlobalOperations ();

         /// \brief Return the global maximum of the value.
         double maximum ( const double val ) const;

         /// \brief Return the global minimum of the value.
         double minimum ( const double val ) const;

         /// \brief Return the global sum of the values.
         double sum ( const double val ) const;

      }; 

   }

}


#endif // _DATA_ACCESS__DISTRIBUTED_APPLICATION_GLOBAL_OPERATIONS__H_
