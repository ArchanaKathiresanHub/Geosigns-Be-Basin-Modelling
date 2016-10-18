//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file SensitivityCalculatorImpl.C
/// @brief This file keeps API implementation of SensitivityCalculator 

// CASA
#include "SensitivityCalculatorImpl.h"
#include "ObsSpace.h"
#include "ObsValueTransformable.h"
#include "RSProxyImpl.h"
#include "RunCaseImpl.h"
#include "RunCaseSet.h"
#include "SUMlibUtils.h"
#include "VarSpace.h"
#include "VarParameter.h"
#include "VarPrmContinuous.h"
#include "VarPrmCategorical.h"

#include <cassert>
#include <sstream>
#include <vector>

// SUMlib
#include "BaseTypes.h"
#include "NumericUtils.h"
#include "Pareto.h"
#include "ParameterPdf.h"
#include "Exception.h"
#include "TornadoSensitivities.h"

namespace casa
{
   typedef std::pair<const SUMlib::Proxy*, double> ProxyWeightPair;
   typedef std::vector<ProxyWeightPair>            ProxyWeightList1D;

   //////////////////////////////////////////////////////////////
   // ParetoSensitivityInfo
   //////////////////////////////////////////////////////////////
   const std::vector< std::pair<const VarParameter *, int > >
   ParetoSensitivityInfo::getVarParametersWithCumulativeImpact( double fraction ) const
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
   double ParetoSensitivityInfo::getSensitivity( const VarParameter * varPrm, int subPrmID ) const
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
   double ParetoSensitivityInfo::getCumulativeSensitivity( const VarParameter * varPrm, int subPrmID ) const
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
   void ParetoSensitivityInfo::add( const VarParameter * varPrm, int subPrmID, double val ) 
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
         // scale also excluded disabled parameters (where min=max)
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
 
            switch ( vprm->variationType() )
            {
               case casa::VarParameter::Continuous:
                  {
                     const casa::VarPrmContinuous * cprm = dynamic_cast<const casa::VarPrmContinuous*>( vprm );
                     const std::vector<bool> & selPrms = cprm->selected();
   
                     for ( size_t j = 0; j < vprm->dimension(); ++j )
                     {
                        if ( selPrms[j] ) { varPrmsPerm.push_back( std::pair< const VarParameter *, int >( vprm, static_cast<int>( j ) ) ); }
                     }
                  }
                  break;

               case casa::VarParameter::Categorical:
               default:
                  {
                     for ( size_t j = 0; j < vprm->dimension(); ++j )
                     {
                        varPrmsPerm.push_back( std::pair< const VarParameter *, int >( vprm, static_cast<int>( j ) ) );
                     }
                  }
                  break;
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


   RSProxyImpl * SensitivityCalculatorImpl::createProxyForTornado( RunCaseSet & cs, const std::vector< std::string> & expNames )
   {
      // create proxy for tornado sensitivity calculation
      std::unique_ptr<RSProxyImpl> proxy( new RSProxyImpl( "TempProxyForSensCalc", *m_varSpace, *m_obsSpace, 
                                                           1, RSProxy::GlobalKriging, false, 0.0, 0.0 ) );
      std::vector< const RunCase *> caseSet;
      for ( auto e : expNames )
      {
         cs.filterByExperimentName( e );
         for ( size_t j = 0; j < cs.size(); ++j ) { caseSet.push_back( cs[j].get() ); }
      }

      // After all preparation and RunCases collecting, here we will calculate proxy
      if ( ErrorHandler::NoError != proxy->calculateRSProxy( caseSet ) ) { throw ErrorHandler::Exception( *(proxy.get()) ); }

      return proxy.release();
   }


   void SensitivityCalculatorImpl::prepareCaseForProxyEvaluation( RunCaseImpl & cs, size_t prmID, size_t subPrmID, double rngValue )
   {
      for ( size_t i = 0; i < m_varSpace->size(); ++i )
      {
         const VarParameter * prm = m_varSpace->parameter( i );

         if ( prmID != i ) { cs.addParameter( prm->baseValue() ); }

         else if ( prm->variationType() == VarParameter::Continuous )
         {
            const VarPrmContinuous * cntPrm = dynamic_cast<const VarPrmContinuous *> ( prm );
            const std::vector<double> & mnPrms = cntPrm->asDoubleArray( cntPrm->minValue()  );
            const std::vector<double> & bsPrms = cntPrm->asDoubleArray( cntPrm->baseValue() );
            const std::vector<double> & mxPrms = cntPrm->asDoubleArray( cntPrm->maxValue()  );

            std::vector<double>         vals( bsPrms.begin(), bsPrms.end() );

            if (      std::abs( rngValue + 1.0 ) < 1e-5 ) { vals[subPrmID] = mnPrms[subPrmID]; }
            else if ( std::abs( rngValue - 1.0 ) < 1e-5 ) { vals[subPrmID] = mxPrms[subPrmID]; }
            else if (                  rngValue  <  0.0 ) { vals[subPrmID] = bsPrms[subPrmID] + (bsPrms[subPrmID] - mnPrms[subPrmID]) * rngValue; }
            else if (                  rngValue  >  0.0 ) { vals[subPrmID] = bsPrms[subPrmID] + (mxPrms[subPrmID] - bsPrms[subPrmID]) * rngValue; } 

            auto it = vals.cbegin();
            cs.addParameter( cntPrm->newParameterFromDoubles( it ) );
         }

         else if ( prm->variationType() == VarParameter::Categorical )
         {
            unsigned int prmVal = static_cast<unsigned int>( subPrmID );
            const VarPrmCategorical * catPrm = dynamic_cast<const VarPrmCategorical *>( prm );
            const std::vector< unsigned int> & vals = catPrm->valuesAsUnsignedIntSortedSet();
            bool found = false;
            for ( auto v : vals ) { if ( v == prmVal ) { found = true; break; } }
            if ( !found ) { throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong categorical value"; }
            cs.addParameter( catPrm->createNewParameterFromUnsignedInt( prmVal ) );
         }

         else { throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Unsupported variable parameter type"; }
      }
   }

   void SensitivityCalculatorImpl::calculatePrmSensitivity( std::vector<TornadoSensitivityInfo> & sensData
                                                          , std::vector<RunCaseImpl>            & css
                                                          , size_t                                prmID
                                                          , size_t                                prmSubID
                                                          )
   {
      if ( css.empty() ) return;
      
      bool continiuosPrm  = m_varSpace->parameter( prmID )->variationType() == VarParameter::Continuous  ? true : false;
      bool categoricalPrm = m_varSpace->parameter( prmID )->variationType() == VarParameter::Categorical ? true : false;

      for ( size_t o = 0, so = 0; o < css[0].observablesNumber(); ++o )
      {
         const ObsValue * obv = css[0].obsValue( o );
         if ( !obv || !obv->isDouble() ) continue;
        
         const Observable * ob = obv->parent();
         if ( !ob ) continue;

         for ( size_t oo = 0; oo < ob->dimension(); ++oo, ++so )
         {
            size_t minValPos = css.size();
            size_t maxValPos = css.size();

            bool hasUndefValue = false;
            std::vector<double> allObsVals( css.size(), 0.0 );
            std::vector<double> allPrmVals( css.size(), 0.0 );

            for ( size_t c = 0; c < css.size(); ++c )
            {
               double csVal = css[c].obsValue( o )->asDoubleArray()[oo];
               allObsVals[ c ] = csVal;

               if (      continiuosPrm  ) { allPrmVals[c] = css[c].parameter( prmID )->asDoubleArray()[prmSubID]; }
               else if ( categoricalPrm ) { allPrmVals[c] = css[c].parameter( prmID )->asInteger(); }
               else                       { throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << "Unsupported variable parameter type"; }

               if ( csVal == UndefinedDoubleValue ) { hasUndefValue = true; continue; }
               else
               {
                  if ( minValPos == css.size() || allObsVals[minValPos] > csVal ) { minValPos = c; }
                  if ( maxValPos == css.size() || allObsVals[maxValPos] < csVal ) { maxValPos = c; }
               }
            }
            sensData[so].addSensitivity( css.front().obsValue( o )->asDoubleArray()[oo]
                                       , css.back( ).obsValue( o )->asDoubleArray()[oo]
                                       , m_varSpace->parameter( prmID ), prmSubID );

            sensData[so].addMinMaxSensitivityInRange( minValPos == css.size() ? UndefinedDoubleValue : allObsVals[minValPos]
                                                    , maxValPos == css.size() ? UndefinedDoubleValue : allObsVals[maxValPos]
                                                    , minValPos == css.size() ? UndefinedDoubleValue : allPrmVals[minValPos]
                                                    , maxValPos == css.size() ? UndefinedDoubleValue : allPrmVals[maxValPos]
                                                    );

            // if there were undefined values - calculate valid intervals
            if ( hasUndefValue ) { sensData[so].calculateAndAddValidRanges( allPrmVals, allObsVals ); }
            else                 { sensData[so].calculateAndAddValidRanges( std::vector<double>(), std::vector<double>() ); }
         }
      }
   }

   std::vector<TornadoSensitivityInfo> SensitivityCalculatorImpl::calculateTornado( RunCaseSet & cs, const std::vector<std::string> & expNames )
   {
      std::unique_ptr<RSProxyImpl> sensProxy( createProxyForTornado( cs, expNames ) );

      std::vector<TornadoSensitivityInfo> returnValue;

      // create base case
      RunCaseImpl baseCase;
      for ( size_t i = 0; i < m_varSpace->size(); ++i ) { baseCase.addParameter( m_varSpace->parameter( i )->baseValue() ); }
      // calculate proxy value for the base case 
      if ( ErrorHandler::NoError != sensProxy->evaluateRSProxy( baseCase ) ) { throw ErrorHandler::Exception( *(sensProxy.get() ) ); }

      // initialize tornado data with base case values
      for ( size_t i = 0; i < baseCase.observablesNumber(); ++i )
      {
         const ObsValue   * obv = baseCase.obsValue( i );
         const Observable * ob = obv->parent();
         if ( !obv->isDouble() || !ob ) continue;
         for ( size_t j = 0; j < ob->dimension(); ++j )
         {
            returnValue.push_back( TornadoSensitivityInfo( ob, static_cast<int>( j ), obv->asDoubleArray()[j] ) );
         }
      }

      // calculate sensitivities first for the continious parameters
      for ( size_t i = 0; i < m_varSpace->size(); ++i )
      {
         const VarParameter * prm = m_varSpace->parameter( i );
         if ( prm->variationType() != VarParameter::Continuous ) { continue; }

         for ( size_t j = 0; j < prm->dimension(); ++j )
         {
            // Calculate 100 RS evaluations on [min:max] parameter interval
            std::vector<RunCaseImpl> css( 101 );
            for ( size_t k = 0; k < css.size(); ++k ) 
            { 
               prepareCaseForProxyEvaluation( css[k], i, j, -1.0 + k * 0.02 );
               if ( ErrorHandler::NoError != sensProxy->evaluateRSProxy( css[k] ) ) { throw ErrorHandler::Exception( *(sensProxy.get() ) ); }
            }
            calculatePrmSensitivity( returnValue, css, i, j );
         }
      }

      // calculate sensitivities then for the categorical parameters
      for ( size_t i = 0; i < m_varSpace->size(); ++i )
      {
         const VarParameter * prm = m_varSpace->parameter( i );
         if ( prm->variationType() != VarParameter::Categorical ) { continue; }
         
         const VarPrmCategorical * catPrm = dynamic_cast<const VarPrmCategorical*>( prm );
         const std::vector<unsigned int> & pvals = catPrm->valuesAsUnsignedIntSortedSet();
         
         for ( auto j : pvals )
         {
            // Calculate RS evaluations for each categorical value
            std::vector<RunCaseImpl> css( 1 );
            prepareCaseForProxyEvaluation( css[0], i, j, 0 );
            if ( ErrorHandler::NoError != sensProxy->evaluateRSProxy( css[0] ) ) { throw ErrorHandler::Exception( *(sensProxy.get() ) ); }
            calculatePrmSensitivity( returnValue, css, i, j );
         }
      }

      // and at the end when we have sensitivities for all paramters, we can calulate relative sensitivities
      for ( size_t i = 0; i < returnValue.size(); ++i ) { returnValue[i].calculateRelativeSensitivities(); }

      return returnValue;
   }

   ////////////////////////////////////////////////////////////////////////////////////////////////
   // Serialization / Deserialization
   //
   // Serialize object to the given stream
   bool SensitivityCalculatorImpl::save( CasaSerializer & sz, unsigned int /* fileVersion */ ) const
   {
      bool ok = true;

      // initial implementation of serialization, must exist in all future versions of serialization
      CasaSerializer::ObjRefID obsID = sz.ptr2id( m_obsSpace );
      CasaSerializer::ObjRefID vspID = sz.ptr2id( m_varSpace );

      ok = ok ? sz.save( obsID, "ObsSpaceID" ) : ok;
      ok = ok ? sz.save( vspID, "VarSpaceID" ) : ok;
      return ok;
   }

   // Serialize object to the given stream
   SensitivityCalculatorImpl::SensitivityCalculatorImpl( CasaDeserializer & dz, const char * objName )
   {
      // read from file object name and version
      unsigned int objVer = version();
      bool ok = dz.checkObjectDescription( typeName(), objName, objVer );

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
