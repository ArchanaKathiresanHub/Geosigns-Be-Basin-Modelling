//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmCategorical.C
/// @brief This file keeps loaders for all possible types of VarPrmCategorical

#include "VarPrmCategorical.h"
#include "VarPrmContinuous.h"

#include "VarPrmSourceRockType.h"

namespace casa
{
   
   std::vector< unsigned int> VarPrmCategorical::valuesAsUnsignedIntSortedSet() const
   {
      std::vector<unsigned int> ret(m_variation.size(), 0 );

      for ( unsigned int i = 0; i < m_variation.size(); ++i ) ret[i] = i;

      return ret;
   }

   SharedParameterPtr VarPrmCategorical::createNewParameterFromUnsignedInt( unsigned int val ) const
   {
      if ( val >= m_variation.size() )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "No such category: " << val <<
            " for categorical parameter: " << m_name;
      }
      
      // inform dependent by sending signal
      m_catDependentVarPrms( m_variation[val].get() );

      return m_variation[val];
   }

   bool VarPrmCategorical::save( CasaSerializer & sz, unsigned int /* version */ ) const
   {
      // register var. parameter with serializer to allow all Parameters objects keep reference after deserializtion
      CasaSerializer::ObjRefID obID = sz.ptr2id( this );
      bool ok = sz.save( obID, "ID" );
 
      // inform dependent by sending signal
      m_serDependentVarPrms( obID );
     
      ok = ok ? sz.save( m_baseVal, "baseValue" ) : ok;
      ok = ok ? sz.save( m_weights, "weights"   ) : ok;

      ok = ok ? sz.save( m_variation.size(), "enumerationSize" ) : ok;

      for ( size_t i = 0; i < m_variation.size() && ok; ++i )
      {
         ok = sz.save( *(m_variation[i].get()), "enumValue" );
      }
     
      ok = ok ? sz.save( m_name, "userGivenName" ) : ok;
      return ok;
   }

   VarPrmCategorical * VarPrmCategorical::load( CasaDeserializer & dz, const char * objName )
   {
      std::string  ot; // object type name
      std::string  on; // object name
      unsigned int vr; // object version

      dz.loadObjectDescription( ot, on, vr );
      if ( on.compare( objName ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "VarPrmCategorical deserialization error, expected VarPrmCategorical with name: " << objName
            << ", but stream gave object with name: " << on;
      }

      // create new variabale parameter object depending on object type name from file
      if ( ot == "VarPrmSourceRockType"  ) { return new VarPrmSourceRockType(  dz, vr ); }
      else
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "VarPrmCategorical deserialization error: Unknown type: " << ot;
      }
      return 0;
   }
   
   // Implements common part of deserialization for continuous variable parameters
   bool VarPrmCategorical::deserializeCommonPart( CasaDeserializer & dz, unsigned int objVer )
   {
      if ( version() < objVer )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError ) <<
            "Version of VarPrmCategorical in file is newer. No forward compatibility!";
      }

      CasaDeserializer::ObjRefID obID;

      // load data necessary to create an object
      bool ok = dz.load( obID, "ID" );

      // register observable with deserializer under read ID to allow Parameters objects keep reference after deserializtion
      if ( ok ) { dz.registerObjPtrUnderID( this, obID ); }

      ok = ok ? dz.load( m_baseVal, "baseValue" ) : ok;
      ok = ok ? dz.load( m_weights, "weights"   ) : ok;

      size_t enumSize;
      ok = ok ? dz.load( enumSize, "enumerationSize" ) : ok;

      for ( size_t i = 0; i < enumSize && ok; ++i )
      {
         SharedParameterPtr prm( Parameter::load( dz, "enumValue" ) );
         m_variation.push_back( prm );
      }

      ok = ok ? dz.load( m_name, "userGivenName" ) : ok;

      return ok;
   }

   void VarPrmCategorical::addDependent( VarPrmContinuous * depPrm )
   {  
      if ( !depPrm ) return;
      
      if ( !dynamic_cast<VarPrmContinuous*>( depPrm ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::SerializationError ) << "Variable parameter: " << depPrm->name()[0] << 
            " can not depends on categorical parameter";
      }

      // to inform dependent parameter that there was a choice of new category value
      m_catDependentVarPrms.connect( boost::bind( &VarPrmContinuous::onCategoryChosen, depPrm, _1 ) );

      // to inform dependent parameter that this parameter was serialized to store dependency
      m_serDependentVarPrms.connect(    boost::bind( &VarPrmContinuous::onSerialization, depPrm, _1 ) );
   }
}
