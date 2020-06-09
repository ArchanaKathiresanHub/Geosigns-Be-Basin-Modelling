//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file DoEGeneratorImpl.cpp
/// @brief This file keeps API implementation of Design of Experiments generator

#include "DoEGeneratorImpl.h"

#include <cassert>
#include <sstream>
#include <vector>

#include "cmbAPI.h"
#include "RunCaseImpl.h"
#include "RunCaseSet.h"
#include "SUMlibUtils.h"
#include "VarPrmContinuous.h"
#include "VarPrmCategorical.h"
#include "VarSpace.h"

// SUMLib includes
#include <BoxBehnken.h>
#include <Case.h>
#include <Exception.h>
#include <FactDesign.h>
#include <HybridMC.h>
#include <OptimisedLHD.h>
#include <ParameterBounds.h>
#include <ScreenDesign.h>
#include <Tornado.h>

namespace casa
{

   std::string DoEGenerator::DoEName( DoEAlgorithm algo )
   {
      switch ( algo )
      {
      case BoxBehnken:           return "BoxBehnken";
      case Tornado:              return "Tornado";
      case PlackettBurman:       return "PlackettBurman";
      case PlackettBurmanMirror: return "PlackettBurmanMirror";
      case FullFactorial:        return "FullFactorial";
      case LatinHypercube:       return "LatinHypercube";
      case BaseCase:             return "BaseCase";
      case SpaceFilling:         return "SpaceFilling";
      default:                   return "Unkown";
      }
   }

   DoEGeneratorImpl::DoEGeneratorImpl( DoEGenerator::DoEAlgorithm algo ) : m_typeOfDoE( algo )
   {
      // throwing exception from constructor is bad idea
      if ( algo < TheFirstDoEAlgo || algo > TheLastDoEAlgo ) throw Exception( ErrorHandler::OutOfRangeValue ) << "Unknown DoE algorithm type was given";
   }

   DoEGeneratorImpl::~DoEGeneratorImpl()
   {
   }

   // Generate set of cases for DoE
   // [in]  varPrmsSet list of influential parameters
   // [out] expSet list of cases for DoE
   // [in]  runsNum number of runs for DoE algorithms which support this parameter
   // return ErrorHandler::NoError on success, error code otherwise
   ErrorHandler::ReturnCode DoEGeneratorImpl::generateDoE( const VarSpace & varPrmsSet, RunCaseSet & doeCaseSet, size_t runsNum, std::string doeName )
   {
      std::vector< std::shared_ptr<RunCase> > expSet;

      try
      {
         SUMlib::Case minCase;
         SUMlib::Case maxCase;
         SUMlib::Case baseCase;

         SUMlib::RealMatrix dummy_variance;
         SUMlib::RealMatrix dummy_disWeights, dummy_catWeights;

         std::vector< SUMlib::IndexList > pCatIndices;

         unsigned int numRuns = static_cast<unsigned int>( runsNum );

         std::vector< bool > selectedPrms;
         sumext::createSUMlibBounds( varPrmsSet, minCase, maxCase, selectedPrms, pCatIndices );
         sumext::createSUMlibPrior( varPrmsSet, baseCase, dummy_variance, dummy_disWeights, dummy_catWeights );


         // create bounds
         const SUMlib::ParameterBounds pBounds( minCase, maxCase, pCatIndices );

         // Let the SUMlib experimental design create a SUMlib case set.
         std::vector<SUMlib::Case> sumCases;

         size_t numOfOrdPrms = 0; // count number of continuous parameters
         for ( size_t i = 0; i < varPrmsSet.numberOfContPrms(); ++i )
         {
            numOfOrdPrms += varPrmsSet.continuousParameter( i )->dimension();
         }

         // create SUMlib object for DoE generation
         if ( SpaceFilling == m_typeOfDoE ) // special case, can extend already existed set of cases
         {
            // collect number of existed SpaceFilling DoE cases:
            size_t numOldRuns = expSet.size();
            const std::vector< std::string > & expNames = doeCaseSet.experimentNames();
            for ( auto nm : expNames )
            {
               if ( nm.find( "SpaceFilling", 0 ) != std::string::npos )
               {
                  doeCaseSet.filterByExperimentName( nm );
                  numOldRuns += doeCaseSet.size();
               }
            }
            doeCaseSet.filterByExperimentName( "" );

            const SUMlib::HybridMC doe( selectedPrms, static_cast<unsigned int>( numOfOrdPrms ), static_cast<unsigned int>( numOldRuns ), static_cast<unsigned int>( runsNum ) );
            const bool replicate = false;
            doe.getCaseSet( pBounds, baseCase, replicate, sumCases );
         }
         else if ( BaseCase == m_typeOfDoE ) // special case
         {
            sumCases.push_back(baseCase);
         }
         else // ordinary case - all new cases from scratch
         {
            // clean case set to prepare for the new DoE generation
            expSet.clear(); // clear array

            // replicate for each categorical value?
            bool replicate = LatinHypercube == m_typeOfDoE ? false : true; // LatinHypercube bit special case, doesn't support replication
            std::unique_ptr<SUMlib::ExpDesign> doe; // SUMLib DoE object

            switch ( m_typeOfDoE )
            {
            case BoxBehnken:           doe.reset( new SUMlib::BoxBehnken(   selectedPrms, static_cast<unsigned int>(numOfOrdPrms)               ) ); break;
            case Tornado:              doe.reset( new SUMlib::Tornado(      selectedPrms, static_cast<unsigned int>(numOfOrdPrms)               ) ); break;
            case PlackettBurman:       doe.reset( new SUMlib::ScreenDesign( selectedPrms, static_cast<unsigned int>(numOfOrdPrms), false, false ) ); break;
            case PlackettBurmanMirror: doe.reset( new SUMlib::ScreenDesign( selectedPrms, static_cast<unsigned int>(numOfOrdPrms), false, true  ) ); break;
            case LatinHypercube:       doe.reset( new SUMlib::OptimisedLHD( selectedPrms, static_cast<unsigned int>(numOfOrdPrms), numRuns      ) ); break;
            case FullFactorial:        doe.reset( new SUMlib::FactDesign(   selectedPrms, static_cast<unsigned int>(numOfOrdPrms)               ) ); break;
            default:
               std::ostringstream oss;
               oss << "Unknown DoE algorithm: " << m_typeOfDoE;
               return reportError( UndefinedValue, oss.str() );
            }
            doe->getCaseSet( pBounds, baseCase, replicate, sumCases );
         }

         for ( size_t c = 0; c < sumCases.size(); ++c )
         {
            addCase( varPrmsSet, expSet, sumCases[c] );
         }

         doeCaseSet.addNewCases( expSet, doeName.empty() ? DoEGenerator::DoEName( m_typeOfDoE ) : doeName );
      }
      catch ( const SUMlib::Exception & e )
      {
         std::ostringstream oss;
         oss << "SUMlib exception caught on DoE case generation: " << e.what();
         return reportError( SUMLibException, oss.str() );
      }
      catch ( const ErrorHandler::Exception & ex )
      {
         return this->ErrorHandler::reportError( ex.errorCode(), ex.what() );
      }

      return NoError;
   }

   // Add case generated by SUMLib to the list of the CASA cases
   // [out] expSet list of CASA cases which will be extended for the new one
   // [in]  cs SUMLib generated case
   void DoEGeneratorImpl::addCase( const VarSpace & varSp, std::vector< std::shared_ptr<RunCase> > & expSet, const SUMlib::Case & cs )
   {
      // create new CASA case
      std::shared_ptr<RunCase> newCase( new RunCaseImpl() );

      sumext::convertCase( cs, varSp, *( newCase.get() ) );

      expSet.push_back( newCase );
   }

   // Serialize object to the given stream
   bool DoEGeneratorImpl::save( CasaSerializer & sz ) const
   {
      // initial implementation of serialization, must exist in all future versions of serialization
      bool ok = sz.save( static_cast<int>(m_typeOfDoE), "TypOfDoE" );
      return ok;
   }

   // Serialize object to the given stream
   DoEGeneratorImpl::DoEGeneratorImpl( CasaDeserializer & dz, const char * objName )
   {
      // read from file object name and version
      unsigned int objVer = version();
      bool ok = dz.checkObjectDescription( typeName(), objName, objVer );

      int doeAlg = -1;
      ok = ok ? dz.load( doeAlg, "TypOfDoE" ) : ok;

      m_typeOfDoE = static_cast<DoEAlgorithm>(doeAlg);

      if ( !ok )
      {
         throw ErrorHandler::Exception( ErrorHandler::DeserializationError )
            << "DoEGeneratorImpl deserialization error";
      }
   }
}
