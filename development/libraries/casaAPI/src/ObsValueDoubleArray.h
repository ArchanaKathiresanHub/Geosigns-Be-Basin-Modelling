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

/// @page CASA_ObsValueDoubleArray Observable value which could be represented as an array of float point values.
/// Suitable for well observables for scalar properties like pressure, temperature and so on for the set of (X,Y,Z)
/// grid points along a well path.
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
      ObsValueDoubleArray( const Observable * parent, const std::vector<double> & val )
         : m_value( val.begin(), val.end() )
         , m_parent( parent ) { ; }

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
      virtual const Observable * parent() const { return m_parent; }

      /// @brief Calculate Mean Squared Error for the observable value if reference value was specified
      /// @return Mean Squared Error
      virtual double MSE() const;

      // The following methods are used for testing
      virtual bool isDouble() const { return true; }
      virtual std::vector<double> asDoubleArray( bool transformed = true ) const { return m_value; }

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 0; }

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual std::string typeName() const { return "ObsValueDoubleArray"; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @return true if it succeeds, false if it fails.
      virtual bool save(CasaSerializer & sz) const;

      /// @brief Create a new ObsValueDoubleScalar instance and deserialize it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      /// @return new ObsValueDoubleScalar instance on susccess, or throw and exception in case of any error
      ObsValueDoubleArray( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:

      std::vector<double>  m_value;    // value itself
      const Observable   * m_parent;   // pointer to the observable description object
   };
}

#endif // CASA_API_OBS_VALUE_DOUBLE_ARRAY_H
