//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file RSProxyImpl.C
/// @brief This file keeps API implementation of Response Surface Proxy


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

#include <cassert>
#include <sstream>

namespace casa
{

RSProxyImpl::RSProxyImpl( const std::string & rspName
                        , const VarSpace & varSp
                        , const ObsSpace & obsSp
                        , size_t rsOrder
                        , RSKrigingType rsKrig
                        , bool autoSearch
                        , double targedR2
                        , double confLevel ) :
        m_name( rspName )
      , m_varSpace( varSp )
      , m_obsSpace( obsSp )
      , m_rsOrder(    rsOrder )
      , m_kriging(    rsKrig )
      , m_autosearch( autoSearch )
      , m_targedR2(   targedR2 )
      , m_confLevel(  confLevel )
{
   assert( m_rsOrder >= 0 && m_rsOrder <= 3 );
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

   // Create a SUMlib cases set
   std::vector<SUMlib::Case>   sumCaseSet( caseSet.size() );

   // convert set of paremetes value for each case from CASA to SUMlib
   for ( std::size_t i = 0; i < caseSet.size(); ++i )
   {
      assert( caseSet[i] != 0 );

      // convert parameters set from RunCase to SUMLib Case
      sumext::convertCase( *(caseSet[i]), sumCaseSet[i] );
   }

   ///////////////////////////
   // create parameters bounds
   const RunCaseImpl * rc = dynamic_cast<const RunCaseImpl *>( caseSet[0] );
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

   sumext::convertCase( minCase, pLowest );
   sumext::convertCase( maxCase, pHighest );

   // will use all parameters as on parameter space
   SUMlib::Partition partition( pLowest.size(), true );

   // collect and convert list of observables to SUMlib TargetCollection
   SUMlib::TargetCollection   targetCollection;
   std::vector< std::vector< bool > >  validCase2Obs; // matrix which defines for which case which observable is valid

   sumext::convertObservablesValue( caseSet, targetCollection, validCase2Obs );   
   assert( !targetCollection.empty() );

   // we do not use parameters transformation yet
   std::vector< std::vector < SUMlib::ParameterTransforms::TransformType > > parameterTransformsDef;

   // Create the proxies, and store shared pointers to them
   m_collection.reset( new SUMlib::CompoundProxyCollection( sumCaseSet, pLowest, pHighest ) );
   try
   {
      m_collection->calculate( targetCollection
                             , validCase2Obs 
                             , static_cast<unsigned int>( m_rsOrder )
                             , m_autosearch
                             , m_targedR2
                             , m_confLevel
                             , partition
                             , parameterTransformsDef );
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
   if ( !m_collection.get() ) return reportError( ErrorHandler::RSProxyError, "Proxy must be calculated before it evalutation" );

   SUMlib::Case  sumCase;

   // convert parameters set from RunCase to SUMLib Case
   sumext::convertCase( cs, sumCase );

   const SUMlib::ProxyValueList & obsVals = m_collection->getProxyValueList( sumCase, static_cast<SUMlib::KrigingType>( m_kriging ) );
   sumext::convertObservablesValue( obsVals, m_obsSpace, cs );

   return NoError;
}


RSProxy::CoefficientsMapList RSProxyImpl::getCoefficientsMapList() const
{
   CoefficientsMapList coefficients;
   m_collection->getCoefficientsMapList( coefficients );
   return coefficients;
}

}
