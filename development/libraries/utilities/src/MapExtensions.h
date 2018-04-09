//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

//std
#include <functional>
#include <map>
#include <limits>
#include <type_traits>

//utilties
#include "NumericFunctions.h"

/// @file MapExtensions.h Contains type definitions for maps using floating points numbers as a key

namespace Utilities{
   /// @class Less Utility to compare floating points numbers
   template <class T>
   class Less : public std::binary_function<T,T,bool>
   {
      static_assert(std::is_floating_point<T>::value, "Class Less needs to be instantiated with floating point type");

      public:
         Less( T epsilon = std::numeric_limits<T>::epsilon() ) : m_epsilon(epsilon) {}

         /// @brief Compare floating points numbers using epsilon as comparison range
         /// @details The epsilon range is scalled to the floating points absolute value
         /// @return true if left is inferior to right outside the scalled epsilon range, false otherwise
         bool operator()( const T& left, const T& right ) const
         {
            return not NumericFunctions::isEqual(left, right, m_epsilon)
               and left < right;
         }
      private:
         const T m_epsilon; ///< Comparison range
   };

   typedef std::map<double,double,Less<double>> doubleKeyMap; ///< A map double to double
   typedef std::map<float,float,Less<float>> floatKeyMap;     ///< A map float to float
}

