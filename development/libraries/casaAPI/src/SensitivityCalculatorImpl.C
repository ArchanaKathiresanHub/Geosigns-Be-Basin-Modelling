//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file SensitivityCalculatorImpl.C
/// @brief This file keeps API implementation of SensetivityCalculator 

// CASA
#include "SensitivityCalculatorImpl.h"
#include "ObsSpace.h"
#include "RSProxyImpl.h"
#include "RunCaseImpl.h"
#include "RunCaseSet.h"
#include "SUMlibUtils.h"
#include "VarSpace.h"
#include "VarParameter.h"

#include <cassert>
#include <sstream>
#include <vector>

// SUMlib
#include "BaseTypes.h"
#include "Pareto.h"
#include "ParameterPdf.h"
#include "Exception.h"
#include "TornadoSensitivities.h"


namespace casa
{
   typedef std::pair<const SUMlib::Proxy*, double> ProxyWeightPair;
   typedef std::vector<ProxyWeightPair>            ProxyWeightList1D;

   //////////////////////////////////////////////////////////////
   // SensitivityCalculator::ParetoSensitivityInfo
   //////////////////////////////////////////////////////////////
   const std::vector< std::pair<const VarParameter *, int > >
   SensitivityCalculator::ParetoSensitivityInfo::getVarParametersWithCumulativeImpact( double fraction ) const
   {
      std::vector< std::pair<const VarParameter *, int> > ret;
      double cumulative = 0.0;
      for ( size_t i = 0; cumulative < fraction && i < m_vprmSens.size(); ++i )
      {
         cumulative += m_vprmSens[i];
         ret.push_back( std::pair< const VarParameter *, int >( m_vprmPtr[i], m_vprmSubID[i] ) );
      }
      return ret;
   }

   // Get the sensitivity of specified VarParameter
   double SensitivityCalculator::ParetoSensitivityInfo::getSensitivity( const VarParameter * varPrm, int subPrmID ) const
   {
      for ( size_t i = 0; i < m_vprmPtr.size(); ++i )
      {
         if ( m_vprmPtr[i] == varPrm && m_vprmSubID[i] == subPrmID )
         {
            return m_vprmSens[i];
         }
      }
      return 0.0;
   }

   // Get the cumulative sensitivity of specified VarParameter
   double SensitivityCalculator::ParetoSensitivityInfo::getCumulativeSensitivity( const VarParameter * varPrm, int subPrmID ) const
   {
      double cumulative = 0.0;
      for ( size_t i = 0; i < m_vprmSens.size(); ++i )
      {
         cumulative += m_vprmSens[i];
         if ( m_vprmPtr[i] == varPrm && m_vprmSubID[i] == subPrmID ) break;
      }
      return cumulative;
   }

   // Add new parameter sensitivity to the list
   void SensitivityCalculator::ParetoSensitivityInfo::add( const VarParameter * varPrm, int subPrmID, double val ) 
   { 
      for ( size_t i = 0; i < m_vprmSens.size() && varPrm; ++i )
      {  // create sorted by sensitivity value array
         if ( m_vprmSens[i] < val )
         {
            m_vprmSens.insert(  m_vprmSens.begin()  + i, val );
            m_vprmSubID.insert( m_vprmSubID.begin() + i, subPrmID );
            m_vprmPtr.insert(   m_vprmPtr.begin()   + i, varPrm );
            varPrm = 0;
         }
      }
      if ( varPrm ) 
      {
         m_vprmSens.push_back(  val );
         m_vprmSubID.push_back( subPrmID );
         m_vprmPtr.push_back(   varPrm );
      }
   }

   //////////////////////////////////////////////////////////////
   // SensitivityCalculator methods
   //////////////////////////////////////////////////////////////
    SensitivityCalculatorImpl::SensitivityCalculatorImpl( const VarSpace * vsp, const ObsSpace * obs ) 
      : m_obsSpace( obs )
      , m_varSpace( vsp )
   {
   }


   ErrorHandler::ReturnCode SensitivityCalculatorImpl::calculatePareto( const RSProxy * proxy, ParetoSensitivityInfo & data )
   {
      try
      {
         const RSProxyImpl * proxySet = dynamic_cast<const RSProxyImpl *>( proxy );
         assert( proxySet );

         const SUMlib::CompoundProxyCollection * proxies = proxySet->getProxyCollection();
         const SUMlib::ParameterSpace          & parSpace = proxies->getParameterSpace();

         // fill and scale variable parameters ranges
         SUMlib::ParameterPdf pdf;
         sumext::convertVarSpace2ParameterPdf( *m_varSpace, parSpace, pdf );
         pdf.scale();

         // fill observables and weights
         // SensitivityInput 2D array of proxy and weight pairs.SensitivityInput[i][j] corresponds to a proxy and weight pair for
         // observable i and well j.
         SUMlib::Pareto::SensitivityInput  proxyMap;

         // create a structure for all observables with non zero SA weight
         for ( size_t i = 0; i < m_obsSpace->size(); ++i )
         {
            // n prop for 1 well
            const Observable * obs = m_obsSpace->observable( i );

            for ( size_t j = 0; j < obs->dimension(); ++j ) // one SA weight for all observable dimension 
            {
               proxyMap.push_back( ProxyWeightList1D() );
               proxyMap.back().push_back( std::pair< const SUMlib::Proxy *, double >( proxies->getProxyList()[i], obs->saWeight() ) );
            }
         }

         std::vector<double>       sensitivity;
         std::vector<double>       cumulatives;
         std::vector<unsigned int> order;

         // Calculate sensitivities, and determine order of decreasing sensitivity
         SUMlib::Pareto pareto;
         pareto.getSensitivities( proxyMap, pdf, sensitivity );
         pareto.normalizeAndSort( sensitivity, order, cumulatives );

         // order is empty if normalization was not possible (i.e. total sensitivity == 0)
         assert( order.empty() || order.size() == sensitivity.size() );

         // create permutation vector to convert linear var. parameter enumeration to VarParameter pointer and sub-parameter ID
         std::vector< std::pair<const VarParameter *, int > > varPrmsPerm;
         for ( size_t i = 0; i < m_varSpace->size(); ++i )
         {
            const VarParameter * vprm = m_varSpace->parameter( i );
            for ( int j = 0; j < vprm->dimension(); ++j )
            {
               varPrmsPerm.push_back( std::pair< const VarParameter *, int >( vprm, j ) );
            }
         }
        
         // fill sensitivities into return data structure
         for ( size_t i = 0; i < order.size(); ++i )
         {
            data.add( varPrmsPerm[order[i]].first, varPrmsPerm[order[i]].second, sensitivity[i] );
         }
      }
      catch ( SUMlib::Exception & e )
      {
         std::ostringstream oss;
         oss << "SUMlib exception caught: " << e.what();
         return reportError( SUMLibException, oss.str() );
      }

      return NoError;
   }
      
   ErrorHandler::ReturnCode SensitivityCalculatorImpl::calculateTornado( RunCaseSet & cs, const std::vector<std::string> & expNames, std::vector<TornadoSensitivityInfo> & returnValue )
   {
      // create proxy for tornado sensitivity calculation
      RSProxyImpl sensProxy( "TempProxyForSensCalc", *m_varSpace, *m_obsSpace, 1, RSProxy::GlobalKriging, false, 0.0, 0.0 );

      std::vector< const RunCase *> caseSet;
      for ( size_t e = 0; e < expNames.size(); ++e )
      {
         cs.filterByExperimentName( expNames[e] );
         for ( size_t j = 0; j < cs.size(); ++j ) { caseSet.push_back( cs[j] ); }
      }

      // After all preparation and RunCases collecting, here we will calculate proxy
      if ( ErrorHandler::NoError != sensProxy.calculateRSProxy( caseSet ) ) return moveError( sensProxy );
      
      // get SUMlib proxies list
      const std::vector<const SUMlib::CompoundProxy *> & tornadoProxiesLst = sensProxy.getProxyCollection()->getProxyList();

      // prepare parameters bounds and PDFs
      const SUMlib::ParameterSpace & parSpace     = sensProxy.getProxyCollection()->getParameterSpace();
      // Create prior parameter PDF
      SUMlib::ParameterPdf priorPar;
      sumext::convertVarSpace2ParameterPdf( *m_varSpace, parSpace, priorPar );

      // Scale pdf to initialize scaledOrdinalBase
      priorPar.scale();

      std::vector< std::pair<const VarParameter *, int> > varPrmList;
      for ( size_t i = 0; i < m_varSpace->size(); ++i )
      {
         const VarParameter * vprm = m_varSpace->parameter( i );
         for ( int j = 0; j < vprm->dimension(); ++j )
         {
            varPrmList.push_back( std::pair< const VarParameter *, int >( vprm, j ) );
         }
      }

      std::vector< std::pair<const Observable *, int> > obsList;
      for ( size_t i = 0; i < m_obsSpace->size(); ++i )
      {
         const Observable * obs = m_obsSpace->observable( i );
         for ( size_t j = 0; j < obs->dimension(); ++j )
         {
            obsList.push_back( std::pair<const Observable *, int>( obs, j ) );
         }
      }

      // Get sensitivities for all observables
      for ( size_t o = 0; o < tornadoProxiesLst.size(); ++o )
      {
         SUMlib::TornadoSensitivities tornado;
         double refObsValue;
         std::vector< std::vector< double > > sensitivities, relSensitivities;
      
         tornado.getSensitivities( tornadoProxiesLst[o], priorPar, refObsValue, sensitivities, relSensitivities );
         returnValue.push_back( TornadoSensitivityInfo( obsList[o].first, obsList[o].second, refObsValue, varPrmList, sensitivities, relSensitivities ) ); 
      }

      return NoError;
   }


   ////////////////////////////////////////////////////////////////////////////////////////////////
   // Serialization / Deserialization
   //
   // Serialize object to the given stream
   bool SensitivityCalculatorImpl::save( CasaSerializer & sz, unsigned int fileVersion ) const
   {
      bool ok = true;

      // initial implementation of serialization, must exist in all future versions of serialization
      if ( fileVersion >= 0 )
      {
         CasaSerializer::ObjRefID obsID = sz.ptr2id( m_obsSpace );
         CasaSerializer::ObjRefID vspID = sz.ptr2id( m_varSpace );

         ok = ok ? sz.save( obsID, "ObsSpaceID" ) : ok;
         ok = ok ? sz.save( vspID, "VarSpaceID" ) : ok;
      }
      return ok;
   }

   // Serialize object to the given stream
   SensitivityCalculatorImpl::SensitivityCalculatorImpl( CasaDeserializer & dz, const char * objName )
   {
      // read from file object name and version
      bool ok = dz.checkObjectDescription( typeName(), objName, version() );

      CasaSerializer::ObjRefID obsID;
      CasaSerializer::ObjRefID vspID;

      ok = ok ? dz.load( obsID, "ObsSpaceID" ) : ok;
      ok = ok ? dz.load( vspID, "VarSpaceID" ) : ok;
      
      m_obsSpace = dz.id2ptr<ObsSpace>( obsID );
      m_varSpace = dz.id2ptr<VarSpace>( vspID );

      if ( !ok )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "SensitivityCalculatorImpl deserialization error";
      }

   }
}
