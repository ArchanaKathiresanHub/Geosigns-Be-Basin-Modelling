//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ObsValueDoubleScalar.h
/// @brief This file keeps definition of the interface class for handling observables value which could be represented\n
/// as float point scalar

#ifndef CASA_API_OBS_VALUE_DOUBLE_SCALAR_H
#define CASA_API_OBS_VALUE_DOUBLE_SCALAR_H

#include "ObsValue.h"

namespace casa
{
   /// @brief Class for keeping observable value as float point scalar value
   class ObsValueDoubleScalar : public ObsValue
   {
   public:
      
      /// @brief Constructor
      /// @param parent Observable object which contains full description of observable
      /// @param val value of observable
      ObsValueDoubleScalar( Observable * parent, double val ) : m_parent( parent ), m_value( val ) { ; }

      /// @brief Copy constructor
      /// @param ov another observable value to be copying
      ObsValueDoubleScalar( const ObsValueDoubleScalar & ov )
      {
         m_value  = ov.m_value;
         m_parent = ov.m_parent;
      }

      /// @brief Destructor
      virtual ~ObsValueDoubleScalar() { ; }

      /// @brief Copy operator
      /// @param otherObs another observable value to be copying
      /// @return reference to the object itself
      ObsValueDoubleScalar & operator = ( const ObsValueDoubleScalar & otherObs )
      {
         m_parent = otherObs.m_parent;
         m_value  = otherObs.m_value;
         return *this;
      }

      /// @brief Get parent observable which define type of observable
      virtual Observable * observable() const { return m_parent; }
      
      /// @brief Get value of the observable as float point scalar
      /// @return observable value
      double value() { return m_value; }

   protected:
      double       m_value;    // value itself
      Observable * m_parent;   // pointer to the observable description object
   };
}

#endif // CASA_API_OBS_VALUE_DOUBLE_SCALAR_H
