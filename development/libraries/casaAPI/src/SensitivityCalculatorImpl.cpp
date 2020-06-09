//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file SensitivityCalculatorImpl.cpp
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
#include <algorithm>
#include <numeric>
#include <limits>

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
   // SensitivityCalculator methods
   //////////////////////////////////////////////////////////////
    SensitivityCalculatorImpl::SensitivityCalculatorImpl( const VarSpace * vsp, const ObsSpace * obs )
      : m_obsSpace( obs )
      , m_varSpace( vsp )
   {
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

         else { throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Unsupported influential parameter type"; }
      }
   }

   void SensitivityCalculatorImpl::calculatePrmSensitivity( std::vector<TornadoSensitivityInfo> & sensData
                                                          , std::vector<RunCaseImpl>            & css
                                                          , size_t                                prmID
                                                          , size_t                                prmSubID
                                                          )
   {
      if ( css.empty() ) return;

      bool continiuosPrm  = m_varSpace->parameter( prmID )->variationType() == VarParameter::Continuous;
      bool categoricalPrm = m_varSpace->parameter( prmID )->variationType() == VarParameter::Categorical;

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
               else                       { throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << "Unsupported influential parameter type"; }

               if ( IsValueUndefined( csVal ) ) { hasUndefValue = true; continue; }
               else
               {
                  if ( minValPos == css.size() || allObsVals[minValPos] > csVal ) { minValPos = c; }
                  if ( maxValPos == css.size() || allObsVals[maxValPos] < csVal ) { maxValPos = c; }
               }
            }
            sensData[so].addSensitivity( css.front().obsValue( o )->asDoubleArray()[oo]
                                       , css.back( ).obsValue( o )->asDoubleArray()[oo]
                                       , m_varSpace->parameter( prmID ), prmSubID );

            sensData[so].addMinMaxSensitivityInRange( minValPos == css.size() ? Utilities::Numerical::IbsNoDataValue : allObsVals[minValPos]
                                                    , maxValPos == css.size() ? Utilities::Numerical::IbsNoDataValue : allObsVals[maxValPos]
                                                    , minValPos == css.size() ? Utilities::Numerical::IbsNoDataValue : allPrmVals[minValPos]
                                                    , maxValPos == css.size() ? Utilities::Numerical::IbsNoDataValue : allPrmVals[maxValPos]
                                                    );

            // if there were undefined values - calculate valid intervals
            if ( hasUndefValue ) { sensData[so].calculateAndAddValidRanges( allPrmVals, allObsVals ); }
            else                 { sensData[so].calculateAndAddValidRanges( std::vector<double>(), std::vector<double>() ); }
         }
      }
   }

   void SensitivityCalculatorImpl::calculateParetoSensitivity( std::vector<double>              & rangeOfPropertyResponse
                                                             , std::vector<std::vector<double>> & propSensitivities
                                                             , std::vector<RunCaseImpl>         & css
                                                             , size_t                             prmID
                                                             , size_t                             prmSubID
                                                             )
   {
      propSensitivities.push_back( std::vector<double>( m_obsSpace->size(), 0.0 ) );

      for ( size_t o = 0; o < m_obsSpace->size(); ++o )
      {
         const Observable * obs = m_obsSpace->observable( o );
         double wgt = obs->uaWeight();

         double minProxy = Utilities::Numerical::IbsNoDataValue;
         double maxProxy = Utilities::Numerical::IbsNoDataValue;

         for ( size_t oo = 0; oo < obs->dimension(); ++oo )
         {
            std::for_each( css.begin(), css.end(), [o, oo, &minProxy, &maxProxy] ( RunCaseImpl & cs )
                                                   {
                                                      double csVal = cs.obsValue( o )->asDoubleArray()[oo];
                                                      if ( !IsValueUndefined( csVal ) )
                                                      {
                                                         minProxy = IsValueUndefined( minProxy ) ? csVal : std::min( csVal, minProxy );
                                                         maxProxy = IsValueUndefined( maxProxy ) ? csVal : std::max( csVal, maxProxy );
                                                      }
                                                   } );
            double dProxy = maxProxy - minProxy;

            if ( dProxy > rangeOfPropertyResponse[o] ) { rangeOfPropertyResponse[o] = dProxy; }
            propSensitivities.back()[o] +=  wgt * dProxy; //actual weighting of the raw sensitivities!
         }
      }
   }

   ErrorHandler::ReturnCode SensitivityCalculatorImpl::calculatePareto( RSProxy * proxy, ParetoSensitivityInfo & data )
   {
      try
      {
         std::vector<double>               rangeOfPropertyResponse( m_obsSpace->size(), std::numeric_limits<double>::epsilon() );
         std::vector<std::vector<double>>  propSensitivities; // Prm x Obs

         // create permutation vector to convert linear var. parameter enumeration to VarParameter pointer and sub-parameter ID
         std::vector< std::pair<const VarParameter *, int > > varPrmsPerm;

         // calculate sensitivities first for the continious parameters
         for ( size_t i = 0; i < m_varSpace->size(); ++i )
         {
            const VarParameter * prm = m_varSpace->parameter( i );

            if ( prm->variationType() == VarParameter::Continuous )
            {
               const std::vector<bool> & selPrms = dynamic_cast<const casa::VarPrmContinuous*>( prm )->selected();
               for ( size_t j = 0; j < prm->dimension(); ++j )
               {
                  if ( !selPrms[j] ) { continue; }
                  // Calculate 100 RS evaluations on [min:max] parameter interval
                  std::vector<RunCaseImpl> css( 101 );
                  for ( size_t k = 0; k < css.size(); ++k )
                  {
                     prepareCaseForProxyEvaluation( css[k], i, j, -1.0 + k * (2.0 / (css.size()-1.0)) );
                     if ( NoError != proxy->evaluateRSProxy( css[k] ) ) { throw ErrorHandler::Exception( *proxy ); }
                  }
                  calculateParetoSensitivity( rangeOfPropertyResponse, propSensitivities, css, i, j );
                  varPrmsPerm.push_back( std::pair< const VarParameter *, int >( prm, static_cast<int>( j ) ) );
               }
            }
            else if ( prm->variationType() == VarParameter::Categorical )
            {
               // Extract all categorical values
               const std::vector<unsigned int> & pvals = dynamic_cast<const VarPrmCategorical*>( prm )->valuesAsUnsignedIntSortedSet();
               // evaluate proxy for each categorica value
               std::vector<RunCaseImpl> css( pvals.size() );
               for ( auto j : pvals )
               {  // Calculate RS evaluations for each categorical value
                  prepareCaseForProxyEvaluation( css[j], i, j, 0 );
                  if ( NoError != proxy->evaluateRSProxy( css[j] ) ) { throw ErrorHandler::Exception( *proxy ); }
               }
               calculateParetoSensitivity( rangeOfPropertyResponse, propSensitivities, css, i, 0 );
               varPrmsPerm.push_back( std::pair< const VarParameter *, int >( prm, 0 ) );
            }
            else { throw Exception( NotImplementedAPI ) << "Unsupported parameter type for Pareto diagram calculation"; }
         }

         // accumulate parameter sensitivity over all observables
         std::vector<double>  sensitivity( propSensitivities.size(), 0.0 );
         for ( size_t i = 0; i < propSensitivities.size(); ++i )
         {
            for ( size_t k = 0; k < propSensitivities[i].size(); ++k ) // go over all observables
            {
               sensitivity[i] += propSensitivities[i][k] / rangeOfPropertyResponse[k];
            }
         }

         // Normalize and sort sensitivities, in order of decreasing sensitivity
         std::vector<unsigned int> order;
         std::vector<double>       cumulatives;
         SUMlib::Pareto pareto;
         pareto.normalizeAndSort( sensitivity, order, cumulatives );

         // order is empty if normalization was not possible (i.e. total sensitivity == 0)
         assert( order.empty() || order.size() == sensitivity.size() );

         // fill sensitivities into return data structure
         for ( size_t i = 0; i < order.size(); ++i )
         {
            data.add( varPrmsPerm[order[i]].first, varPrmsPerm[order[i]].second, sensitivity[i] );
         }
      }
      catch( const ErrorHandler::Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }

      return NoError;
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
               prepareCaseForProxyEvaluation( css[k], i, j, -1.0 + k * (2.0 / (css.size()-1.0)) );
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

      // and at the end when we have sensitivities for all parameters, we can calulate relative sensitivities
      for ( size_t i = 0; i < returnValue.size(); ++i ) { returnValue[i].calculateRelativeSensitivities(); }

      return returnValue;
   }

   ////////////////////////////////////////////////////////////////////////////////////////////////
   // Serialization / Deserialization
   //
   // Serialize object to the given stream
   bool SensitivityCalculatorImpl::save( CasaSerializer & sz ) const
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
