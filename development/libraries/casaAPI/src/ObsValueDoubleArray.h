//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ObsValueDoubleArray.h
/// @brief This file keeps definition of the interface class for handling array of observable values which could be represented
/// as float point array

#ifndef CASA_API_OBS_VALUE_DOUBLE_ARRAY_H
#define CASA_API_OBS_VALUE_DOUBLE_ARRAY_H

#include "ObsValue.h"

namespace casa
{
   /// @brief Class for keeping observable value as an array of float point values
   class ObsValueDoubleArray : public ObsValue
   {
   public:
      /// @brief Create a new ObsValueDoubleArray object. 
      /// @param parent Observable object which contains full description of observable
      /// @param val value of observable
      static ObsValueDoubleArray * createNewInstance( const Observable * parent, 
                                                      const std::vector<double> & val ) { return new ObsValueDoubleArray( parent, val ); }

      /// @brief Constructor
      /// @param parent Observable object which contains full description of observable
      /// @param val value of observable
      ObsValueDoubleArray( const Observable * parent, const std::vector<double> & val ) : 
         m_parent( parent ), 
         m_value( val.begin(), val.end() )
      { ; }

      /// @brief Copy constructor
      /// @param ov another observable value to be copying
      ObsValueDoubleArray( const ObsValueDoubleArray & ov )
      {
         m_value  = ov.m_value;
         m_parent = ov.m_parent;
      }

      /// @brief Destructor
      virtual ~ObsValueDoubleArray() { ; }

      /// @brief Copy operator
      /// @param otherObs another observable value to be copying
      /// @return reference to the object itself
      ObsValueDoubleArray & operator = ( const ObsValueDoubleArray & otherObs )
      {
         m_parent = otherObs.m_parent;
         m_value  = otherObs.m_value;
         return *this;
      }

      /// @brief Get parent observable which define type of observable
      virtual const Observable * observable() const { return m_parent; }
 
      /// @brief Calculate Mean Squared Error for the observable value if reference value was specified
      /// @return Mean Squared Error
      virtual double MSE() const;
     
      // The following methods are used for testing  
      virtual bool isDouble() const { return true; }
      virtual std::vector<double> asDoubleArray() const { return m_value; }

   protected:
      std::vector<double>  m_value;    // value itself
      const Observable   * m_parent;   // pointer to the observable description object
   };

}

#endif // CASA_API_OBS_VALUE_DOUBLE_ARRAY_H
