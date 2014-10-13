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

#include <vector>

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
      virtual const Observable * observable() const = 0;

      /// @brief Calculate Root Mean Squared Error for the observable value if reference value was specified
      /// MSE is calculated using this formula:
      /// @f$ MSE \equiv {\frac{1}{N} \sum\nolimits_{i=1}^{i=N} {\left( {\frac{Y_i-M_i \left( {x_j } \right)}{\sigma_i }} \right)^2} } @f$
      /// @return Root Mean Squared Error
      virtual double MSE() const = 0;

      // The following methods are used for converting observables values from CASA to SUMLib 
      virtual bool isDouble() const = 0;
      virtual std::vector<double> doubleValue() const = 0;

   protected:
      ObsValue() { ; }
   
   private:
      ObsValue & operator = ( const ObsValue & otherObs );
   };
}

#endif // CASA_API_OBSVALUE_H
