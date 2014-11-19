//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarSpaceImpl.C
/// @brief This file keeps API implementation for variable parameters set manager. 

#include "VarSpaceImpl.h"

#include <cassert>

namespace casa
{
   VarSpaceImpl::~VarSpaceImpl()
   {
      for ( size_t i = 0; i < m_catPrms.size(); ++i ) delete m_catPrms[i];
      for ( size_t i = 0; i < m_disPrms.size(); ++i ) delete m_disPrms[i];
      for ( size_t i = 0; i < m_cntPrms.size(); ++i ) delete m_cntPrms[i];

      m_catPrms.clear();
      m_disPrms.clear();
      m_cntPrms.clear();
   }

   ErrorHandler::ReturnCode VarSpaceImpl::addParameter( VarPrmCategorical * prm )
   {
      if ( prm ) { m_catPrms.push_back( prm ); }
      else       { return reportError( UndefinedValue, "VarSpaceImpl::addParameter() no parameter given" ); }
      return NoError;
   }

   ErrorHandler::ReturnCode VarSpaceImpl::addParameter( VarPrmContinuous * prm )
   {
      if ( prm ) { m_cntPrms.push_back( prm ); }
      else       { return reportError( UndefinedValue, "VarSpaceImpl::addParameter() no parameter given" ); }
      return NoError;
   }

   // Add a new discrete parameter
   ErrorHandler::ReturnCode VarSpaceImpl::addParameter( VarPrmDiscrete * prm )
   {
      if ( prm ) { m_disPrms.push_back( prm ); }
      else       { return reportError( UndefinedValue, "VarSpaceImpl::addParameter() no parameter given" ); }
      return NoError;
   }


   // Serialize object to the given stream
   bool VarSpaceImpl::save( CasaSerializer & sz, unsigned int fileVersion ) const
   {
      bool ok = true;

      // initial implementation of serialization, must exist in all future versions of serialization
      if ( fileVersion >= 0 )
      { 
         // register VarSpace object with serializer to allow other objects to keep reference after deserializtion
         CasaSerializer::ObjRefID vspID = sz.ptr2id( this );
         ok = ok ? sz.save( vspID, "ID" ) : ok;

         // Categorical parameters
         size_t setSize = m_catPrms.size();

         ok = ok ? sz.save( setSize, "CatPrmsSetSize" ) : ok;

         for ( size_t i = 0; i < setSize && ok; ++i )
         {
            ok = ok ? sz.save( *(m_catPrms[i]), "CatVarParameter" ) : ok;
         }

         // Discrete parameters
         setSize = m_disPrms.size();

         ok = ok ? sz.save( setSize, "DisPrmsSetSize" ) : ok;

         for ( size_t i = 0; i < setSize && ok; ++i )
         {
            ok = ok ? sz.save( *(m_disPrms[i]), "DisVarParameter" ) : ok;
         }

         // Continuous parameters
         setSize = m_cntPrms.size();

         ok = ok ? sz.save( setSize, "CntPrmsSetSize" ) : ok;

         for ( size_t i = 0; i < setSize && ok; ++i )
         {
            ok = ok ? sz.save( *(m_cntPrms[i]), "CntVarParameter" ) : ok;
         }
      }
      return ok;
   }

   // Create a new instance and deserialize it from the given stream
   VarSpaceImpl::VarSpaceImpl( CasaDeserializer & dz, const char * objName )
   {
      // read from file object name and version
      bool ok = dz.checkObjectDescription( typeName(), objName, version() );
 
      CasaDeserializer::ObjRefID vspID;

      // load data necessary to create an object

      // register VarSpace with deserializer under saved ID to allow other objects to keep reference after deserialization
      ok = ok ? dz.load( vspID, "ID" ) : ok;
      if ( ok ) { dz.registerObjPtrUnderID( this, vspID ); }

      // Categorical parameters
      size_t setSize;

      ok = ok ? dz.load( setSize, "CatPrmsSetSize" ) : ok;

      for ( size_t i = 0; i < setSize && ok; ++i )
      {
         VarPrmCategorical * newVar = VarPrmCategorical::load( dz, "CatVarParameter" );
         assert( newVar );

         ok = ok ? (NoError == addParameter( newVar )) : ok;
      }

      // Discrete parameters
      ok = ok ? dz.load( setSize, "DisPrmsSetSize" ) : ok;

      for ( size_t i = 0; i < setSize && ok; ++i )
      {
         VarPrmDiscrete * newVar = VarPrmDiscrete::load( dz, "DisVarParameter" );
         assert( newVar );

         ok = ok ? (NoError == addParameter( newVar )) : ok;
      }

      // Continuous parameters
      ok = ok ? dz.load( setSize, "CntPrmsSetSize" ) : ok;

      for ( size_t i = 0; i < setSize && ok; ++i )
      {
         VarPrmContinuous * newVar = VarPrmContinuous::load( dz, "CntVarParameter" );
         assert( newVar );

         ok = ok ? (NoError == addParameter( newVar )) : ok;
      }

      if ( !ok ) throw Exception( DeserializationError ) << "VarSpaceImpl deserialization error";
   }
}