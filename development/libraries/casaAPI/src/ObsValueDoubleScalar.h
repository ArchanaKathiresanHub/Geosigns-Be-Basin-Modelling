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
/// @brief This file keeps definition of the interface class for handling observables value which could be represented
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
      /// @brief Create a new ObsValueDoubleScalar object. 
      /// @param parent Observable object which contains full description of observable
      /// @param val value of observable
      static ObsValueDoubleScalar * createNewInstance( const Observable * parent, double val ) { return new ObsValueDoubleScalar( parent, val ); }

      /// @brief Constructor. Use create() static method to create new instance of this class
      /// @param parent Observable object which contains full description of observable
      /// @param val value of observable
      ObsValueDoubleScalar( const Observable * parent, double val ) : m_parent( parent ), m_value( val ) { ; }

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
      virtual const Observable * parent() const { return m_parent; }
      
      /// @brief Calculate Mean Squared Error for the observable value if reference value was specified
      /// @return Mean Squared Error
      virtual double MSE() const;

      /// @brief Get value of the observable as float point scalar
      /// @return observable value
      double value() const { return m_value; }

      // The following methods are used for testing  
      virtual bool isDouble() const { return true; }
      virtual std::vector<double> asDoubleArray( bool /* transformed */ = true ) const { return std::vector<double>( 1, value() ); }

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 0; }

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "ObsValueDoubleScalar"; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;
      
      /// @brief Create a new ObsValueDoubleScalar instance and deserialize it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      /// @return new ObsValueDoubleScalar instance on susccess, or throw and exception in case of any error
      ObsValueDoubleScalar( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:

      const Observable * m_parent;   // pointer to the observable description object
      double             m_value;    // value itself
   
   private:
   };
}

#endif // CASA_API_OBS_VALUE_DOUBLE_SCALAR_H
