//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file RSProxyImpl.cpp
/// @brief This file keeps API implementation of Response Surface Proxy

// CASA
#include "CasaDeserializer.h"
#include "Parameter.h"
#include "VarParameter.h"
#include "VarPrmCategorical.h"
#include "RSProxyImpl.h"
#include "RunCaseImpl.h"
#include "SUMlibUtils.h"

// SUMlib includes
#include <Case.h>
#include <CompoundProxyCollection.h>
#include <Exception.h>
#include <ParameterTransforms.h>

// STL
#include <cassert>
#include <sstream>

namespace casa
{

RSProxyImpl::RSProxyImpl( const std::string & rspName
                        , const VarSpace    & varSp
                        , const ObsSpace    & obsSp
                        , size_t              rsOrder
                        , RSKrigingType       rsKrig
                        , bool                autoSearch
                        , double              targedR2
                        , double              confLevel
                        )
                        : m_name(       rspName    )
                        , m_varSpace(  &varSp      )
                        , m_obsSpace(  &obsSp      )
                        , m_kriging(    rsKrig     )
                        , m_autosearch( autoSearch )
                        , m_confLevel(  confLevel  )
{
   m_rsOrder = rsOrder;
   assert( m_rsOrder <= 3 );
   assert( !m_name.empty() );
}

RSProxyImpl::~RSProxyImpl()
{
   ;
}

// Calculate polynomial coefficients for the given cases set
ErrorHandler::ReturnCode RSProxyImpl::calculateRSProxy( const std::vector<const RunCase*> & caseSet )
{
   if ( caseSet.empty() ) return reportError( RSProxyError, "Can not build response surface approximation for zero size case set" );


   std::vector< const RunCase*> filteredCaseSet;
   removeDuplicated( caseSet, filteredCaseSet );

   // Create a SUMlib cases set
   std::vector<SUMlib::Case>   sumCaseSet( filteredCaseSet.size() );

   // convert set of paremetes value for each case from CASA to SUMlib
   for ( std::size_t i = 0; i < filteredCaseSet.size(); ++i )
   {
      assert( filteredCaseSet[i] != 0 );

      // convert parameters set from RunCase to SUMLib Case
      sumext::convertCase( *(filteredCaseSet[i]), *m_varSpace, sumCaseSet[i] );
   }

   ///////////////////////////
   // create parameters bounds
   const RunCase * rc = filteredCaseSet[0];
   assert( rc != 0 );

   RunCaseImpl minCase;
   RunCaseImpl maxCase;

   std::vector<SUMlib::IndexList> catIndices;

   //////////////////////////
   // go over all parameters and collect min/max values
   for ( size_t j = 0; j < rc->parametersNumber(); ++j )
   {
      const SharedParameterPtr prm( rc->parameter( j ) );
      assert( prm.get() != 0 );
      assert( prm->parent() != 0 );

      minCase.addParameter( prm->parent()->minValue() );
      maxCase.addParameter( prm->parent()->maxValue() );

      // collect categorical parameters enumeration set
      if ( prm->parent()->variationType() == VarParameter::Categorical )
      {
         const std::vector<unsigned int> & valsSet = dynamic_cast<const VarPrmCategorical*>(prm->parent())->valuesAsUnsignedIntSortedSet();
         assert( !valsSet.empty() );
         catIndices.push_back( SUMlib::IndexList( valsSet.begin(), valsSet.end() ) );
      }
   }

   SUMlib::Case pLowest;
   SUMlib::Case pHighest;

   sumext::convertCase( minCase, *m_varSpace, pLowest );
   sumext::convertCase( maxCase, *m_varSpace, pHighest );

   // will use all parameters as on parameter space
   SUMlib::Partition partition( pLowest.size(), true );

   // collect and convert list of observables to SUMlib TargetCollection
   SUMlib::TargetCollection       targetCollection;
   std::vector<std::vector<bool>> validCase2Obs; // matrix which defines for which case which observable is valid

   sumext::convertObservablesValue( filteredCaseSet, targetCollection, validCase2Obs );
   assert( !targetCollection.empty() );

   // we do not use parameters transformation yet
   std::vector<std::vector<SUMlib::ParameterTransforms::TransformType>> parameterTransformsDef;

   // Create the proxies, and store shared pointers to them
   m_collection.reset( new SUMlib::CompoundProxyCollection( sumCaseSet, pLowest, pHighest ) );
   try
   {
      m_collection->calculate( targetCollection
                             , validCase2Obs
                             , static_cast<unsigned int>( m_rsOrder )
                             , m_autosearch
                             , 0.95 // Note: R2 is calculated in CASA itself
                             , m_confLevel
                             , partition
                             , parameterTransformsDef );

      m_collection->getCoefficientsMapList( m_coefficients );
   }
   catch ( SUMlib::InvalidTransforms & e )
   {
      std::ostringstream msg;
      msg << "Invalid transforms specified for the following observable-parameter combinations:\n";
      for ( size_t i = 0; i < e.getNumInvalidTransforms(); ++i )
      {
         msg << "\n";
         msg << " Observable: " << e.getObservableIndex( i ) << ", VarParameter: " << e.getParameterIndex( i ) << "\n";
         msg << "=> Reason: " << e.getReason( i ) << ".\n";
      }
      return reportError( ErrorHandler::RSProxyError, std::string( "SUMlib error: " ) + msg.str() );
   }

   return NoError;
}

// Calculate values of observables for given set of parameters
ErrorHandler::ReturnCode RSProxyImpl::evaluateRSProxy( RunCase & cs )
{
   if ( !m_collection ) return reportError( ErrorHandler::RSProxyError, "Proxy must be calculated before its evaluation" );

   SUMlib::Case  sumCase;

   // convert parameters set from RunCase to SUMLib Case
   sumext::convertCase( cs, *m_varSpace, sumCase );

   const SUMlib::ProxyValueList & obsVals = m_collection->getProxyValueList( sumCase, static_cast<SUMlib::KrigingType>( m_kriging ) );
   sumext::convertObservablesValue( obsVals, *m_obsSpace, cs );

   return NoError;
}


int RSProxyImpl::polynomialOrder() const
{
   // search for maximal order of polynomial, order is defined by
   // the size of coefficient array per parameter value
   size_t ord = 0;
   for ( size_t j = 0; j < m_coefficients.size(); ++j )
   {
      const RSProxy::CoefficientsMap & cmap = m_coefficients[j];

      std::vector<int> numCoefPerOrd;

      // get polynomial order
      for ( RSProxy::CoefficientsMap::const_iterator it = cmap.begin(); it != cmap.end(); ++it )
      {
         const std::vector< unsigned int > & prmsLst = it->first;
         ord = ord < prmsLst.size() ? prmsLst.size() : ord;
      }
   }
   return static_cast<int>( ord );
}


const RSProxy::CoefficientsMapList & RSProxyImpl::getCoefficientsMapList() const
{
   return m_coefficients;
}

// Serialize object to the given stream
bool RSProxyImpl::save( CasaSerializer & sz ) const
{
   bool ok = sz.save( m_name, "ProxyName" );

   CasaSerializer::ObjRefID obsID = sz.ptr2id( m_obsSpace );
   CasaSerializer::ObjRefID vspID = sz.ptr2id( m_varSpace );

   ok = ok ? sz.save( obsID, "ObsSpaceID" ) : ok;
   ok = ok ? sz.save( vspID, "VarSpaceID" ) : ok;

   ok = ok ? sz.save( m_rsOrder,                   "Order"               ) : ok;
   ok = ok ? sz.save( static_cast<int>(m_kriging), "Kriging"             ) : ok;
   ok = ok ? sz.save( m_autosearch,                "AutoSearch"          ) : ok;
   ok = ok ? sz.save( m_confLevel,                 "ConfLevel"           ) : ok;
   ok = ok ? sz.save( *m_collection.get(),         "CompProxyCollection" ) : ok;

   return ok;
}

// Create a new instance and deserialize it from the given stream
RSProxyImpl::RSProxyImpl( CasaDeserializer & dz, const char * objName )
{
   // read from file object name and version
   std::string  objNameInFile;
   std::string  objType;
   unsigned int objVer;

   bool ok = dz.loadObjectDescription( objType, objNameInFile, objVer );
   if ( objType.compare( this->typeName() ) || objNameInFile.compare( objName ) )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "Deserialization error. Can not load object: " << objName;
   }

   if ( version() < objVer )
   {
      throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
         << "Version of object in file is newer. No forward compatibility!";
   }

   ok = ok ? dz.load( m_name, "ProxyName" ) : ok;

   CasaSerializer::ObjRefID obsID;
   CasaSerializer::ObjRefID vspID;

   ok = ok ? dz.load( obsID, "ObsSpaceID" ) : ok;
   ok = ok ? dz.load( vspID, "VarSpaceID" ) : ok;

   m_obsSpace = dz.id2ptr<ObsSpace>( obsID );
   m_varSpace = dz.id2ptr<VarSpace>( vspID );

   ok = ok ? dz.load( m_rsOrder, "Order" ) : ok;

   int krType;
   ok = ok ? dz.load( krType, "Kriging" ) : ok;
   m_kriging = static_cast<RSProxy::RSKrigingType>( krType );

   ok = ok ? dz.load( m_autosearch, "AutoSearch" ) : ok;

   if( objVer < 1 )
   {
     double dummy;
     ok = ok ? dz.load( dummy, "TargedR2" ) : ok;
   }
   ok = ok ? dz.load( m_confLevel, "ConfLevel" ) : ok;

   m_collection.reset( new SUMlib::CompoundProxyCollection() );
   ok = ok ? dz.load( *m_collection.get(), "CompProxyCollection" ) : ok;

   // get coefficients again
   if ( ok ) m_collection->getCoefficientsMapList( m_coefficients );

   if ( !ok ) throw Exception( DeserializationError ) << "RSProxyImpl deserialization error";
}


void RSProxyImpl::removeDuplicated( const std::vector<const RunCase*> & caseSet, std::vector< const RunCase*> & filteredCaseSet )
{
   if ( caseSet.empty() ) return;
   filteredCaseSet.push_back( caseSet[0] );

   for ( size_t i = 1; i < caseSet.size(); ++i )
   {
      bool found = false;
      for ( size_t j = 0; j < filteredCaseSet.size() && !found; ++j )
      {
         if ( *(caseSet[i]) == *(filteredCaseSet[j] ) ) found = true;
      }
      if ( !found ) filteredCaseSet.push_back( caseSet[i] );
   }
}

}
