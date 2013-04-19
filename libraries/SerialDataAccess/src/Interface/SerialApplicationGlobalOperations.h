#ifndef _DATA_ACCESS__SERIAL_APPLICATION_GLOBAL_OPERATIONS__H_
#define _DATA_ACCESS__SERIAL_APPLICATION_GLOBAL_OPERATIONS__H_


#include "Interface/ApplicationGlobalOperations.h"


namespace DataAccess {

   namespace Interface {

      /// \brief Class for handling global operations.
      ///
      /// The idea is to have a serial and distributed version of this class
      /// Which ever kind of application we are running the correct class will 
      /// be allocated. 
      class SerialApplicationGlobalOperations : public ApplicationGlobalOperations {

      public :

         ~SerialApplicationGlobalOperations ();

         /// \brief Return the global maximum of the value.
         double maximum ( const double val ) const;

         /// \brief Return the global minimum of the value.
         double minimum ( const double val ) const;

         /// \brief Return the global sum of the values.
         double sum ( const double val ) const;

      }; 

   }

}


#endif // _DATA_ACCESS__SERIAL_APPLICATION_GLOBAL_OPERATIONS__H_
