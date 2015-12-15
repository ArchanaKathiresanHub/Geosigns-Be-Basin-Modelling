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

#include "CasaSerializer.h"
#include "CasaDeserializer.h"
// STL
#include <vector>

namespace casa
{
   class Observable; // parent object

   /// @brief Base class for describing interface to abstract observable value. Value could be different types.
   /// The simplest is just double value
   class ObsValue : public CasaSerializable
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
      /// @brief Can be this observable vale be represented as array of doubles?
      /// @return true if it can be represented as array of doubles, false otherwise
      virtual bool isDouble() const = 0;

      /// @brief  Converts this object to a double array.
      /// @return array of doubles which represent value of this observable
      virtual std::vector<double> asDoubleArray() const = 0;

      /// @brief Create a new observable value instance and deserialize it from the given stream
      /// @param dz input stream
      /// @param objName expected object name
      /// @return new observable value instance on susccess, or throw and exception in case of any error
      static ObsValue * load( CasaDeserializer & dz, const char * objName );

   protected:
      ObsValue() { ; }

   private:
      ObsValue & operator = ( const ObsValue & otherObs );
   };
}

#endif // CASA_API_OBSVALUE_H
