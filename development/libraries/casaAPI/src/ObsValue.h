//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ObsValue.h
/// @brief This file keeps declaration of the interface class for handling observables value

#ifndef CASA_API_OBSVALUE_H
#define CASA_API_OBSVALUE_H

namespace casa
{
   class Observable; // parent object

   /// @brief Base class for describing interface to abstract observable value. Value could be different types.
   /// The simplest is just double value
   class ObsValue
   {
   public:
      /// @brief Destructor
      virtual ~ObsValue() { ; }

      /// @brief Get parent observable which define type of observable
      virtual Observable * observable() const = 0;

      // The following methods are used for testing  
      virtual bool isDouble() = 0;
      virtual double doubleValue() = 0;

   protected:
      ObsValue() { ; }
   
   private:
      ObsValue & operator = ( const ObsValue & otherObs );
   };
}

#endif // CASA_API_OBSVALUE_H
