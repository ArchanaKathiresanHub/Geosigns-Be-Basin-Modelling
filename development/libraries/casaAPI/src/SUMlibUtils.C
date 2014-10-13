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
#include "VarSpaceImpl.h"

// SUMlib includes
#include <Case.h>

#include <vector>
#include <cassert>

// CASA -> SUMlib case conversion
void sumext::convertCase( const casa::RunCase & crc, SUMlib::Case  & sc )
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
               const std::vector<double> & prmVals = prm->asDoubleArray();
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

// SUMlib -> CASA case conversion
void sumext::convertCase( const SUMlib::Case  & sc, const casa::VarSpace & vp, casa::RunCase & crc )
{
   casa::RunCaseImpl & newCase = dynamic_cast<casa::RunCaseImpl &>( crc );
   assert( newCase.parametersNumber() == 0 ); // must be empty case!

   const casa::VarSpaceImpl & varSpace = dynamic_cast<const casa::VarSpaceImpl &>( vp );

   const std::vector<double>       & sumCntArray = sc.continuousPart();
   const std::vector<unsigned int> & sumCatArray = sc.categoricalPart();

   std::vector<double>::const_iterator cit = sumCntArray.begin();

   // go over all parameters in scenario and convert parameters

   // Continuous parameters first!
   for ( size_t i = 0; i < varSpace.numberOfContPrms(); ++i )
   {
      assert( cit != sumCntArray.end() );
      SharedParameterPtr  newPrm = varSpace.continuousParameter( i )->newParameterFromDoubles( cit );
      newCase.addParameter( newPrm );
   }

   // Then categorical parameters
   for ( size_t i = 0; i < varSpace.numberOfCategPrms(); ++i )
   {
      newCase.addParameter( varSpace.categoricalParameter( i )->createNewParameterFromUnsignedInt( sumCatArray[ i ] ) );
   }
}

// CASA -> SUMlib observables conversion
void sumext::convertObservablesValue( const std::vector<const casa::RunCase*> & caseSet
                                    , SUMlib::TargetCollection          & targetsSet 
                                    , std::vector< std::vector<bool> >  & matrValidObs )
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
         obsNum += obv->doubleValue().size();
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
            const std::vector<double> & vals = obv->doubleValue();

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

