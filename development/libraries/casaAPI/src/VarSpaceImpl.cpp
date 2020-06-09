//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file VarSpaceImpl.cpp
/// @brief This file keeps API implementation for influential parameters set manager.

#include "VarSpaceImpl.h"

#include <cassert>

namespace casa
{
   VarSpaceImpl::~VarSpaceImpl()
   {
      for ( size_t i = 0; i < m_prms.size(); ++i ) delete m_prms[i];
      m_prms.clear();

      m_catPrms.clear();
      m_disPrms.clear();
      m_cntPrms.clear();
   }

   ErrorHandler::ReturnCode VarSpaceImpl::addParameter( VarParameter * prm )
   {
      if ( prm )
      {
         switch( prm->variationType() ) // also add parameter to the list depending on parameter type
         {
            case VarParameter::Continuous:  m_cntPrms.push_back( dynamic_cast<VarPrmContinuous* >( prm ) ); break;
            case VarParameter::Categorical: m_catPrms.push_back( dynamic_cast<VarPrmCategorical*>( prm ) ); break;
            case VarParameter::Discrete:    m_disPrms.push_back( dynamic_cast<VarPrmDiscrete   *>( prm ) ); break;

            default:
               return reportError( UndefinedValue, "Unknown influential parameter type: " );
               break;
         }
         m_prms.push_back( prm ); // add parameter to the list of all var parameters
      }
      else
      {
         return reportError( UndefinedValue, "VarSpaceImpl::addParameter() no parameter given" );
      }
      return NoError;
   }

   // Serialize object to the given stream
   bool VarSpaceImpl::save( CasaSerializer & sz ) const
   {
      bool ok = true;

      // register VarSpace object with serializer to allow other objects to keep reference after deserializtion
      CasaSerializer::ObjRefID vspID = sz.ptr2id( this );
      ok = ok ? sz.save( vspID, "ID" ) : ok;

      // Just all parameters
      size_t setSize = m_prms.size();

      ok = ok ? sz.save( setSize, "VarPrmsSetSize" ) : ok;
      for ( size_t i = 0; i < setSize && ok; ++i )
      {
         int prmType = static_cast<int>( m_prms[i]->variationType() );
         ok = ok ? sz.save( prmType,      "PrmVariationType" ) : ok;
         ok = ok ? sz.save( *(m_prms[i]), "VarParameter"     ) : ok;
      }

      return ok;
   }

   // Create a new instance and deserialize it from the given stream
   VarSpaceImpl::VarSpaceImpl( CasaDeserializer & dz, const char * objName )
   {
      // read from file object name and version
      unsigned int objVer = version();
      bool ok = dz.checkObjectDescription( typeName(), objName, objVer );

      CasaDeserializer::ObjRefID vspID;

      // load data necessary to create an object

      // register VarSpace with deserializer under saved ID to allow other objects to keep reference after deserialization
      ok = ok ? dz.load( vspID, "ID" ) : ok;
      if ( ok ) { dz.registerObjPtrUnderID( this, vspID ); }

      size_t setSize;

      ok = ok ? dz.load( setSize, "VarPrmsSetSize" ) : ok;

      for ( size_t i = 0; i < setSize && ok; ++i )
      {
         int prmType;
         ok = ok ? dz.load( prmType, "PrmVariationType" ) : ok;
         if ( ok )
         {
            VarParameter * newVar = NULL;

            switch( static_cast< VarParameter::Type >( prmType ) )
            {
               case VarParameter::Continuous:  newVar = VarPrmContinuous::load(  dz, "VarParameter" ); break;
               case VarParameter::Categorical: newVar = VarPrmCategorical::load( dz, "VarParameter" ); break;
               case VarParameter::Discrete:    newVar = VarPrmDiscrete::load(    dz, "VarParameter" ); break;
               default: assert( false );
            }
            ok = newVar ? ok : false;
            ok = ok ? (NoError == addParameter( newVar )) : ok;
         }
      }

      if ( !ok ) throw Exception( DeserializationError ) << "VarSpaceImpl deserialization error";
   }
}

