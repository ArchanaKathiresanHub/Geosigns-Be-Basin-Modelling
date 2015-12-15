//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file SUMlibUtils.C
/// @brief This file keeps implementation of the set of utility functions to convert data SUMlib <-> CASA

#include "SUMlibUtils.h"

#include "Observable.h"
#include "ObsSpaceImpl.h"
#include "ObsValue.h"
#include "Parameter.h"
#include "RunCaseImpl.h"
#include "SensitivityCalculatorImpl.h"
#include "VarSpaceImpl.h"

// SUMlib includes
#include <Case.h>
#include <ParameterPdf.h>

#include <vector>
#include <cassert>

///////////////////////////////////////////////////////////////////////////////
// CASA -> SUMlib case conversion
void sumext::convertCase( const casa::RunCase & crc, const casa::VarSpace & vp, SUMlib::Case  & sc )
{
   const casa::RunCaseImpl & rci = dynamic_cast<const casa::RunCaseImpl &>( crc );

   std::vector<double>       sumCntArray; // continuous parameters value
   std::vector<unsigned int> sumCatArray; // categorical parameters value

   // go over all parameters
   for ( size_t i = 0; i < rci.parametersNumber(); ++i )
   {
      const SharedParameterPtr prm = rci.parameter( i );
      assert( prm );
      assert( prm->parent() );

      switch ( prm->parent()->variationType() )
      {
         case casa::VarParameter::Continuous:
            {
               const casa::VarPrmContinuous * cntPrm = dynamic_cast<const casa::VarPrmContinuous *>( prm->parent() );

               const std::vector<double> & prmVals = cntPrm->asDoubleArray( prm );
               sumCntArray.insert( sumCntArray.end(), prmVals.begin(), prmVals.end() );
            }
            break;

         case casa::VarParameter::Categorical:
            {
               int prmIVal = prm->asInteger();
               sumCatArray.push_back( prmIVal );
            }
            break;

         default: assert( 0 ); break;
      }
   }

   sc.setContinuousPart( sumCntArray );  // set continuous parameters values to SUMlib::Case
   sc.setCategoricalPart( sumCatArray ); // set categorical parameters values to SUMlib::Case
}


///////////////////////////////////////////////////////////////////////////////
// SUMlib -> CASA case conversion
void sumext::convertCase( const SUMlib::Case  & sc, const casa::VarSpace & vp, casa::RunCase & crc )
{
   casa::RunCaseImpl & newCase = dynamic_cast<casa::RunCaseImpl &>( crc );
   assert( newCase.parametersNumber() == 0 ); // must be empty case!

   const casa::VarSpaceImpl & varSpace = dynamic_cast<const casa::VarSpaceImpl &>( vp );

   const std::vector<double>       & sumCntArray = sc.continuousPart();
   const std::vector<unsigned int> & sumCatArray = sc.categoricalPart();

   std::vector< double      >::const_iterator cntIt = sumCntArray.begin();
   std::vector< unsigned int>::const_iterator catIt = sumCatArray.begin();

   // go over all parameters in scenario and convert parameters
   for ( size_t i = 0; i < varSpace.size(); ++i )
   {
      const casa::VarParameter * prm = varSpace.parameter( i );
      switch ( prm->variationType() )
      {
         case casa::VarParameter::Continuous:
            {
               const casa::VarPrmContinuous * cntPrm = dynamic_cast<const casa::VarPrmContinuous *>( prm );
               assert( cntIt != sumCntArray.end() );

               SharedParameterPtr newPrm = cntPrm->newParameterFromDoubles( cntIt );
               newCase.addParameter( newPrm );
            }
            break;

         case casa::VarParameter::Categorical:
            {
               const casa::VarPrmCategorical * catPrm = dynamic_cast<const casa::VarPrmCategorical *>( prm );
               assert( catIt != sumCatArray.end() );
               newCase.addParameter( catPrm->createNewParameterFromUnsignedInt( *catIt ) );
               ++catIt;
            }
            break;

         case casa::VarParameter::Discrete:
         default:
            throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << "Not implemented variable parameter type: " << prm->variationType(); 
            break;
      }
   }
}


///////////////////////////////////////////////////////////////////////////////
// CASA -> SUMlib observables conversion
void sumext::convertObservablesValue( const std::vector<const casa::RunCase*> & caseSet
                                    , SUMlib::TargetCollection                & targetsSet 
                                    , std::vector< std::vector<bool> >        & matrValidObs )
{
   assert( targetsSet.empty() );
   assert( !caseSet.empty() );

   matrValidObs.clear();

   size_t obsNum = 0;
   size_t caseNum = caseSet.size();

   // count number of observables value for the first case
   const casa::RunCaseImpl * rc0 = dynamic_cast<const casa::RunCaseImpl *>( caseSet[0] );
   for ( size_t j = 0; j < rc0->observablesNumber(); ++j )
   {
      // get observable value and check is it double? 
      const casa::ObsValue * obv = rc0->obsValue( j );
      assert( obv );
      if ( obv->isDouble() )
      {
         obsNum += obv->asDoubleArray().size();
      }
   }
   
   assert( obsNum > 0 );

   // allocate arrays and initialize them with default values
   for ( size_t i = 0; i < caseSet.size(); ++i )
   {
      matrValidObs.push_back( std::vector<bool>( obsNum, false ) );
   }

   for ( size_t i = 0; i < obsNum; ++i )
   {
      targetsSet.push_back( SUMlib::TargetSet( caseSet.size(), 0.0 ) );
   }

   // CASA:  case[i].observables[j]  -> SUMLib observable[j].cases[i]
   //
   // had array of cases, each case keeps list of observables value
   // SUMlib need array of observables with arrays of cases value
   // again go over all cases
   
   for ( size_t i = 0; i < caseSet.size(); ++i )
   {
      const casa::RunCaseImpl * rc = dynamic_cast<const casa::RunCaseImpl *>( caseSet[i] );
      assert( rc != 0 );
      
      obsNum = 0;
      // for each case go over all observable
      for ( size_t j = 0; j < rc->observablesNumber(); ++j )
      {
         // get observable value and check is it double? 
         const casa::ObsValue * obv = rc->obsValue( j );
         assert( obv );
         assert( obv->observable() );
   
         const casa::Observable * osb = obv->observable();
         if ( obv->isDouble() )
         {
            // push values of observable to array of targets
            const std::vector<double> & vals = obv->asDoubleArray();

            for ( size_t k = 0; k < vals.size(); ++k )
            {
               targetsSet[obsNum][i] = vals[k];
               matrValidObs[i][obsNum] = true;

               ++obsNum;
               assert( obsNum <= targetsSet.size() );
            }
         }
      }
   }
}


///////////////////////////////////////////////////////////////////////////////
// SUMlib -> CASA observables conversion
void sumext::convertObservablesValue( const SUMlib::ProxyValueList &  valList, const casa::ObsSpace & obsDef, casa::RunCase & cs )
{
   casa::RunCaseImpl        & rc  = dynamic_cast<casa::RunCaseImpl &>( cs );
   const casa::ObsSpaceImpl & obs = dynamic_cast<const casa::ObsSpaceImpl &> ( obsDef );

   SUMlib::ProxyValueList::const_iterator vit = valList.begin();
   
   // must be empty RunCase without any observable value
   assert( rc.observablesNumber() == 0 );

   // go over all observables definition and create observable value for each observable from array of doubles
   for ( size_t i = 0; i < obs.size(); ++i )
   {
      assert( vit != valList.end() ); // simple check if array has yet unused values

      casa::ObsValue * obsVal = obs[i]->createNewObsValueFromDouble( vit ); // create new observable value (can throw!)
      if ( obsVal ) rc.addObsValue( obsVal ); // add new observable value to the RunCase
   }
}


///////////////////////////////////////////////////////////////////////////////
// Create SUMlib bounds.
void sumext::createSUMlibBounds( const casa::VarSpace           & varSp
                               , SUMlib::Case                   & lowCs
                               , SUMlib::Case                   & highCs
                               , std::vector<bool>              & selectedPrms
                               , std::vector<SUMlib::IndexList> & catIndices
                               )
{
   const casa::VarSpaceImpl & varSpace = dynamic_cast<const casa::VarSpaceImpl &>( varSp );

   casa::RunCaseImpl lowRCs;
   casa::RunCaseImpl uprRCs;

   selectedPrms.clear(); // clean mask array   
   catIndices.clear();   // clean container for categorical values

   for (size_t i = 0; i < varSpace.size(); ++i)
   {

      lowRCs.addParameter( varSpace.parameter( i )->minValue() );
      uprRCs.addParameter( varSpace.parameter( i )->maxValue() );
      switch ( varSpace.parameter(i)->variationType() )
      {
         case casa::VarParameter::Continuous:
         {
            const casa::VarPrmContinuous * prm = dynamic_cast<const casa::VarPrmContinuous*>( varSpace.parameter( i ) );
            const std::vector<bool> & selPrms = prm->selected();
            selectedPrms.insert(selectedPrms.end(), selPrms.begin(), selPrms.end());
         }
         break;

      case casa::VarParameter::Categorical:
         {
            const casa::VarPrmCategorical * prm = dynamic_cast<const casa::VarPrmCategorical*>(varSpace.parameter(i));
            selectedPrms.push_back( prm->selected() );

            const std::vector<unsigned int> & valsSet = prm->valuesAsUnsignedIntSortedSet();
            assert(!valsSet.empty());

            catIndices.push_back(SUMlib::IndexList(valsSet.begin(), valsSet.end()));
         }
         break;

      default:
         assert(0);  // other cases not implemented yet
         break;
      }
   }

   sumext::convertCase( lowRCs, varSp, lowCs );
   sumext::convertCase( uprRCs, varSp, highCs );
}

///////////////////////////////////////////////////////////////////////////////
// Create SUMlib prior info. Set base case
void sumext::createSUMlibPrior( const casa::VarSpace & varSpace
                              , SUMlib::Case & pBase
                              , SUMlib::RealMatrix & variance
                              , SUMlib::RealMatrix & disWeights
                              , SUMlib::RealMatrix & catWeights
                              )
{
   // Create a CASA case for the base, and create an array containing all standard deviations. The standard
   // deviations and the variance matrix are used for continuous variable parameters only. For discrete and categorical 
   // varibal parameters it return the weights.
  
   SUMlib::RealVector stdDevs;
   casa::RunCaseImpl  baseRCs;

   for ( size_t i = 0; i < varSpace.size(); ++i )
   {
      // set base value
      baseRCs.addParameter( varSpace.parameter( i )->baseValue() );

      switch (varSpace.parameter(i)->variationType())
      {
         case casa::VarParameter::Continuous:
            {
               const casa::VarPrmContinuous * prm = dynamic_cast<const casa::VarPrmContinuous*>( varSpace.parameter( i ) );

               // calculate standard deviation value:
               const std::vector<double> & prmStdDev = prm->stdDevs();
               stdDevs.insert( stdDevs.end(), prmStdDev.begin(), prmStdDev.end() );
            }
            break;

         case casa::VarParameter::Discrete:
            {
               const casa::VarPrmDiscrete * prm = dynamic_cast<const casa::VarPrmDiscrete*>( varSpace.parameter( i ) );
               std::vector< double > weights = prm->weights();

               // If no weights are specified, create a vector with weights all equal to 1.
               if (weights.empty())
               {
                  const std::vector<SharedParameterPtr> & valsSet = prm->valuesSet();
                  weights.assign( valsSet.size(), 1.0 );
               }
               disWeights.push_back( weights );
            }
            break;

         // process categorical parameters
         case casa::VarParameter::Categorical:
            {
               const casa::VarPrmCategorical * prm = dynamic_cast<const casa::VarPrmCategorical *>(varSpace.parameter( i ));

               std::vector< double > weights = prm->weights();
               // If no weights are specified, create a vector with weights all equal to 1.
               if ( weights.empty() )
               {
                  const std::vector<unsigned int> & prmIndSet = prm->valuesAsUnsignedIntSortedSet();
                  weights.assign( prmIndSet.size(), 1.0 );
               }
               catWeights.push_back(weights);
            }
            break;   
      }
   }
   // Convert the base case.
   sumext::convertCase( baseRCs, varSpace, pBase );
   
   // Convert the standard deviations to a variance matrix.
   for (std::size_t i = 0; i < stdDevs.size(); ++i)
   {
      variance.push_back(SUMlib::RealVector(stdDevs.size(), 0.0));
      variance.back()[i] = stdDevs[i] * stdDevs[i];
   }
}


///////////////////////////////////////////////////////////////////////////////
// Create a SUMlib ParameterPdf. CASA->SUMlib
void sumext::convertVarSpace2ParameterPdf( const casa::VarSpace & varSpace, const SUMlib::ParameterSpace & pSpace, SUMlib::ParameterPdf & pdf )
{
   // Initialize SUMlib cases and variance.
   SUMlib::Case pBase;
   SUMlib::Case pAbsLow;
   SUMlib::Case pAbsHigh;
   std::vector< SUMlib::IndexList > pCatIndices;

   SUMlib::RealMatrix variance;
   SUMlib::RealMatrix disWeights, catWeights;

   std::vector<bool> selectedPrms;

   // Create SUMlib prior info and outer bounds
   sumext::createSUMlibBounds( varSpace, pAbsLow, pAbsHigh, selectedPrms, pCatIndices );
   sumext::createSUMlibPrior( varSpace, pBase, variance, disWeights, catWeights );

   // Create SUMlib PDF.
   pdf.initialise( pSpace, pAbsLow, pAbsHigh, pBase, variance, disWeights, catWeights );
}

////////////////////////////////////////////////////////////////////////////////
/// Creates custom-made box constraints that can be passed directly to SUMlib::McmcBase.
/// @param [in]  proxyVs          Proxy varSpace
/// @param [out] boxConstraints   Custom-made box contraints stored as a SUMlib::ParameterBounds instance
///
////////////////////////////////////////////////////////////////////////////////
void sumext::createBoxConstraints( const casa::VarSpace & proxyVs, const casa::VarSpace & mcmcVs, SUMlib::ParameterBounds & boxConstraints )
{
   // Properly scale the mcmc bounds and convert the discrete parts to continuous parts.
   // Determine intersection of categorical values.
   std::vector< double > sumConScaledLow;  // continuous values, for the low case, scaled between [ -1, +1 ]
   std::vector< double > sumConScaledHigh; // continuous values, for the high case, scaled between [ -1, +1 ]

   std::vector< double > sumDisScaledLow;  // discrete values, for the low case, scaled between [ -1, +1 ]
   std::vector< double > sumDisScaledHigh; // discrete values, for the high case, scaled between [ -1, +1 ]
   
   std::vector< unsigned int > sumCatLow;  // categorical values, for the low case
   std::vector< unsigned int > sumCatHigh; // categorical values, for the high case
   
   std::vector< SUMlib::IndexList > sumCatIndices; // shared categorical values between proxyVs and mcmcVs

   assert( proxyVs.size() == mcmcVs.size() );

   // process continuous parameters 
   assert( proxyVs.numberOfContPrms() == mcmcVs.numberOfContPrms() );

   for ( size_t i = 0; i < proxyVs.numberOfContPrms(); ++i )
   {
      const casa::VarPrmContinuous * proxyPrm = proxyVs.continuousParameter( i );

      const std::vector<double> & proxyMinVals = proxyPrm->minValue()->asDoubleArray();
      const std::vector<double> & proxyMaxVals = proxyPrm->maxValue()->asDoubleArray();

      const casa::VarPrmContinuous * mcmcPrm = mcmcVs.continuousParameter( i );
 
      const std::vector<double> & mcmcMinVals = mcmcPrm->minValue()->asDoubleArray();
      const std::vector<double> & mcmcMaxVals = mcmcPrm->maxValue()->asDoubleArray();

      assert( proxyMinVals.size() == mcmcMaxVals.size() );

      // scale values to [-1:1] range
      for ( size_t j = 0; j < proxyMinVals.size(); ++j )
      {
         double min_proxy = proxyMinVals[j];
         double max_proxy = proxyMaxVals[j];
         double min_mcmc  = mcmcMinVals[j];
         double max_mcmc  = mcmcMaxVals[j];
         double range_proxy = max_proxy - min_proxy;
         
         assert( range_proxy > 0.0 );

         double scaledLow  = -1.0 + 2 * ( min_mcmc - min_proxy ) / range_proxy;
         double scaledHigh = -1.0 + 2 * ( max_mcmc - min_proxy ) / range_proxy;

         sumConScaledLow.push_back( scaledLow );
         sumConScaledHigh.push_back( scaledHigh );
      }
   }

   // process discrete parameters 
   assert( proxyVs.numberOfDiscrPrms() == mcmcVs.numberOfDiscrPrms() );

   for ( size_t i = 0; i < proxyVs.numberOfDiscrPrms(); ++i )
   {
      const casa::VarPrmDiscrete * proxyPrm = proxyVs.discreteParameter( i );

      const std::vector<double> & proxyMinVals = proxyPrm->minValue()->asDoubleArray();
      const std::vector<double> & proxyMaxVals = proxyPrm->maxValue()->asDoubleArray();

      const casa::VarPrmDiscrete * mcmcPrm = mcmcVs.discreteParameter( i );
 
      const std::vector<double> & mcmcMinVals = mcmcPrm->minValue()->asDoubleArray();
      const std::vector<double> & mcmcMaxVals = mcmcPrm->maxValue()->asDoubleArray();

      assert( proxyMinVals.size() == mcmcMaxVals.size() );

      // scale values to [-1:1] range
      for ( size_t j = 0; j < proxyMinVals.size(); ++j )
      {
         double min_proxy = proxyMinVals[j];
         double max_proxy = proxyMaxVals[j];
         double min_mcmc  = mcmcMinVals[j];
         double max_mcmc  = mcmcMaxVals[j];
         double range_proxy = max_proxy - min_proxy;
         
         assert( range_proxy > 0.0 );

         double scaledLow  = -1.0 + 2 * ( min_mcmc - min_proxy ) / range_proxy;
         double scaledHigh = -1.0 + 2 * ( max_mcmc - min_proxy ) / range_proxy;

         sumDisScaledLow.push_back( scaledLow );
         sumDisScaledHigh.push_back( scaledHigh );
      }
   }

   // process categorical parameters
   assert( proxyVs.numberOfCategPrms() == mcmcVs.numberOfCategPrms() );

   for ( size_t i = 0; i < proxyVs.numberOfCategPrms(); ++i )
   {
      const std::vector< unsigned int> & proxyCategories = proxyVs.categoricalParameter( i )->valuesAsUnsignedIntSortedSet();
      const std::vector< unsigned int> & mcmcCategories  = mcmcVs.categoricalParameter(  i )->valuesAsUnsignedIntSortedSet();
      std::vector< unsigned int > sharedCategories;
      std::set_intersection( proxyCategories.begin(), proxyCategories.end(), mcmcCategories.begin(), mcmcCategories.end(),
                             std::back_inserter( sharedCategories ) );

      sumCatIndices.push_back( sharedCategories );

      sumCatLow.push_back( sumCatIndices.back().front() );
      sumCatHigh.push_back( sumCatIndices.back().back() );
   }

   // Create the final SUMlib box constraints.
   sumConScaledLow.insert(  sumConScaledLow.end(),  sumDisScaledLow.begin(),  sumDisScaledLow.end() );
   sumConScaledHigh.insert( sumConScaledHigh.end(), sumDisScaledHigh.begin(), sumDisScaledHigh.end() );
   
   const SUMlib::Case sumCaseLow ( sumConScaledLow,  std::vector< int >(), sumCatLow  );
   const SUMlib::Case sumCaseHigh( sumConScaledHigh, std::vector< int >(), sumCatHigh );

   boxConstraints.initialise( sumCaseLow, sumCaseHigh, sumCatIndices );
}

