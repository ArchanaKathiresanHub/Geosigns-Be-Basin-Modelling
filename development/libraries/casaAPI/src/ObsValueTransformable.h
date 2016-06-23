//                                                                     
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ObsValueTransformable.h
/// @brief This file keeps definition of the interface class for handling array of float point observable values which 
///        calculated from another array of float point observable values using transformation algorithm from parent Observable object

#ifndef CASA_API_OBS_VALUE_TRANSFORMABLE_H
#define CASA_API_OBS_VALUE_TRANSFORMABLE_H

#include "ObsValueDoubleArray.h"
#include "Observable.h"

namespace casa
{
   /// @brief Class for keeping observable value as an array of float point values
   class ObsValueTransformable : public ObsValue
   {
   public:
      /// @brief Create a new ObsValueTransformable object. 
      /// @param parent Observable object which contains full description of observable
      /// @param val value of observable
      static ObsValueTransformable * createNewInstance( const Observable          * parent
                                                      , const std::vector<double> & val 
                                                      )
      { return new ObsValueTransformable( parent, val ); }

      /// @brief Constructor
      /// @param parent Observable object which contains full description of observable
      /// @param val value of observable
      ObsValueTransformable( const Observable * parent, const std::vector<double> & val );

      /// @brief Copy constructor
      /// @param ov another observable value to be copying
      ObsValueTransformable( const ObsValueTransformable & ov ) : m_value(      ov.m_value      )
                                                                , m_transfVals( ov.m_transfVals )
                                                                , m_parent(     ov.m_parent     )
                                                                {;}
      /// @brief Destructor
      virtual ~ObsValueTransformable() {;}

      /// @brief Copy operator
      /// @param ov another observable value to be copying
      /// @return reference to the object itself
      ObsValueTransformable & operator = ( const ObsValueTransformable & ov )
      {
         m_parent     = ov.m_parent;
         m_value      = ov.m_value;
         m_transfVals = ov.m_transfVals;
         return *this;
      }

      /// @brief Get parent observable which define type of observable
      virtual const Observable * parent() const { return m_parent; }
 
      /// @brief Calculate Mean Squared Error for the observable value if reference value was specified
      /// @return Mean Squared Error
      virtual double MSE() const;
     
      // The following methods are used for testing  
      virtual bool isDouble() const { return true; }

      /// @brief Get observable value as double array
      /// @param transformed which value for observable shold be returned - transformed is set to default
      /// @return if transformed is true - transformed value, otherwise the original one
      virtual std::vector<double> asDoubleArray( bool transformed = true ) const { return transformed ? m_transfVals : m_value; }

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 0; }

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "ObsValueTransformable"; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;

      /// @brief Create a new ObsValueDoubleScalar instance and deserialize it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      /// @return new ObsValueDoubleScalar instance on susccess, or throw and exception in case of any error
      ObsValueTransformable( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
      std::vector<double>  m_value;      // value itself
      std::vector<double>  m_transfVals; // transformed values
      const Observable   * m_parent;     // pointer to the observable description object
   };
}

#endif // CASA_API_OBS_VALUE_DOUBLE_ARRAY_H
