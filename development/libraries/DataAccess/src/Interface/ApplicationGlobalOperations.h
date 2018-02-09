//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _DATA_ACCESS__APPLICATION_GLOBAL_OPERATIONS__H_
#define _DATA_ACCESS__APPLICATION_GLOBAL_OPERATIONS__H_


namespace DataAccess {

   namespace Interface {

      /// \brief Class for handling global operations.
      ///
      /// The idea is to have a serial and distributed version of this class
      /// Which ever kind of application we are running the correct class will 
      /// be allocated. 
      class ApplicationGlobalOperations {

      public :

         virtual ~ApplicationGlobalOperations () = default;

         /// \brief Return the global maximum of the value.
         virtual double maximum ( const double val ) const = 0;

         /// \brief Return the global minimum of the value.
         virtual double minimum ( const double val ) const = 0;

         /// \brief Return the global sum of the values.
         virtual double sum ( const double val ) const = 0;

      }; 

   }

}


#endif // _DATA_ACCESS__APPLICATION_GLOBAL_OPERATIONS__H_
