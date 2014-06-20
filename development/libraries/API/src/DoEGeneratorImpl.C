//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file DoEGeneratorImpl.C
/// @brief This file keeps API implementation of Design of Experiments generator

#include "DoEGeneratorImpl.h"

#include <cassert>
#include <sstream>
#include <vector>

#include "cmbAPI.h"
#include "RunCaseImpl.h"
#include "RunCaseSetImpl.h"
#include "VarPrmContinuous.h"
#include "VarPrmCategorical.h"
#include "VarSpaceImpl.h"

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
   switch( algo )
   {
      case BoxBehnken:           return "BoxBehnken";
      case Tornado:              return "Tornado";
      case PlackettBurman:       return "PlackettBurman";
      case PlackettBurmanMirror: return "PlackettBurmanMirror";
      case FullFactorial:        return "FullFactorial";
      case LatinHypercube:       return "LatinHypercube";
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
// [in]  varPrmsSet list of variable parameters
// [out] expSet list of cases for DoE
// [in]  runsNum number of runs for DoE algorithms which support this parameter
// return ErrorHandler::NoError on success, error code otherwise
ErrorHandler::ReturnCode DoEGeneratorImpl::generateDoE( const VarSpace & varPrmsSet, RunCaseSet & doeCaseSet, size_t runsNum )
{
   const std::vector< bool > selectedPrms( varPrmsSet.size(), true );
   std::vector<RunCase *> expSet;

   try
   {
      SUMlib::Case minCase;
      SUMlib::Case maxCase;
      SUMlib::Case baseCase;
      std::vector< SUMlib::IndexList > pCatIndices;
   
      createBounds( varPrmsSet, minCase, maxCase, pCatIndices );
      setBaseCase( varPrmsSet, baseCase );

      // create bounds. No categorical values for current implementation
      const SUMlib::ParameterBounds pBounds( minCase, maxCase, std::vector<SUMlib::IndexList>() );

      // Let the SUMlib experimental design create a SUMlib case set.
      std::vector<SUMlib::Case> sumCases;

      // create SUMlib object for DoE generation
      if ( SpaceFilling == m_typeOfDoE ) // special case, can extend already existed set of cases
      {
         const SUMlib::HybridMC doe( selectedPrms, static_cast<unsigned int>( varPrmsSet.numberOfContPrms() ),
            static_cast<unsigned int>( expSet.size() ), static_cast<unsigned int>( runsNum ) );
         const bool replicate = false;
         doe.getCaseSet( pBounds, baseCase, replicate, sumCases );
      }
      else // ordinary case - all new cases from scratch
      {
         // clean case set to prepare for the new DoE generation
         for ( size_t i = 0; i < expSet.size(); ++i ) if ( expSet[i] ) delete dynamic_cast<RunCaseImpl*>( expSet[i] ); // delete all existed cases one by one
         expSet.clear(); // clear array
         
         // replicate for each categorical value?
         bool replicate = LatinHypercube == m_typeOfDoE ? false : true; // LatinHypercube bit special case, doesn't support replication
         std::auto_ptr<SUMlib::ExpDesign> doe; // SUMLib DoE object
         
         switch (  m_typeOfDoE )
         {
         case BoxBehnken:
            doe.reset( new SUMlib::BoxBehnken( selectedPrms, static_cast<unsigned int>( varPrmsSet.numberOfContPrms() ) ) );
            break;

         case Tornado:
            doe.reset( new SUMlib::Tornado( selectedPrms, static_cast<unsigned int>( varPrmsSet.numberOfContPrms() ) ) );
            break;

         case PlackettBurman:
            doe.reset( new SUMlib::ScreenDesign( selectedPrms, static_cast<unsigned int>( varPrmsSet.numberOfContPrms() ), false, false ) );
            break;

         case PlackettBurmanMirror:
            doe.reset( new SUMlib::ScreenDesign( selectedPrms, static_cast<unsigned int>( varPrmsSet.numberOfContPrms() ), false, true ) );
            break;

         case LatinHypercube:
            doe.reset( new SUMlib::OptimisedLHD( selectedPrms, static_cast<unsigned int>( varPrmsSet.numberOfContPrms( ) ),
               static_cast<unsigned int>( runsNum ) ) );
            break;

         case FullFactorial:
            doe.reset( new SUMlib::FactDesign( selectedPrms, static_cast<unsigned int>( varPrmsSet.numberOfContPrms() ) ) );
            break;

         default:
            std::ostringstream oss;
            oss << "Unknown DoE algorithm: " << m_typeOfDoE;
            return reportError( UndefinedValue, oss.str() );
         }
         doe->getCaseSet( pBounds, baseCase, replicate, sumCases );
      }

      for ( std::vector<SUMlib::Case>::const_iterator c = sumCases.begin(); c != sumCases.end(); ++c )
      {
         addCase( varPrmsSet, expSet, *c );
      }

      RunCaseSetImpl & doeCases = dynamic_cast<RunCaseSetImpl &>( doeCaseSet );
      doeCases.addNewCases( expSet, DoEGenerator::DoEName( m_typeOfDoE ) );
   }
   catch ( const ::Exception & e )
   {
         std::ostringstream oss;
         oss << "SUMlib exception caught on DoE case generation: " << e.what();
         return reportError( SUMLibException, oss.str() );
   }

   return NoError;
}

// Create SUMlib bounds.
// [in] varSp       VarSpace
// [out] lowCs      SUMlib lower bounds
// [out] highCs     SUMlib upper bounds
// [out] catIndices indexes of categorical parameters
void DoEGeneratorImpl::createBounds( const VarSpace & varSp, SUMlib::Case & lowCs, SUMlib::Case & highCs, std::vector<SUMlib::IndexList> & catIndices )
{
   const VarSpaceImpl & varSpace = dynamic_cast<const VarSpaceImpl &>( varSp );
   
   // process continuous parameters set  
   std::vector<double> minCntPrms( varSpace.numberOfContPrms() );
   std::vector<double> maxCntPrms( varSpace.numberOfContPrms() );

   for ( size_t i = 0; i < varSpace.numberOfContPrms(); ++i )
   {
      minCntPrms[ i ] = varSpace.continuousParameter( i )->minValueAsDouble();
      maxCntPrms[ i ] = varSpace.continuousParameter( i )->maxValueAsDouble();
   }
   lowCs.setContinuousPart( minCntPrms );
   highCs.setContinuousPart( maxCntPrms );

   // process categorical parameters set  
   // clean container for categorical values
   catIndices.clear();

   for ( size_t i = 0; i < varSpace.numberOfCategPrms(); ++i )
   {
      const std::vector<unsigned int> & valsSet = varSpace.categoricalParameter( i )->valuesAsUnsignedIntSortedSet( ); 
      catIndices.push_back( SUMlib::IndexList( valsSet.begin(), valsSet.end() ) );
   }
}

// Set base case
// [in] varSp       VarSpace
// [out] baseCs     base case
void DoEGeneratorImpl::setBaseCase( const VarSpace & varSp, SUMlib::Case & baseCs )
{
   const VarSpaceImpl & varSpace = dynamic_cast<const VarSpaceImpl &>( varSp );

   // process continuous parameters set
   std::vector<double> baseCntPrms( varSpace.numberOfContPrms() );

   for ( size_t i = 0; i < baseCntPrms.size( ); ++i )
   {
      baseCntPrms[ i ] = varSpace.continuousParameter( i )->baseValueAsDouble();
   }
   baseCs.setContinuousPart( baseCntPrms );
}

// Add case generated by SUMLib to the list of the CASA cases
// [out] expSet list of CASA cases which will be extended for the new one
// [in]  cs SUMLib generated case
void DoEGeneratorImpl::addCase( const VarSpace & varSp, std::vector<RunCase*> & expSet, const SUMlib::Case & cs )
{
   const VarSpaceImpl & varSpace = dynamic_cast<const VarSpaceImpl &>( varSp );

   // create new CASA case
   std::auto_ptr<RunCaseImpl> newCase( new RunCaseImpl() );

   const std::vector<double>       & sumCntArray = cs.continuousPart();
   const std::vector<unsigned int> & sumCatArray = cs.categoricalPart();

   // go over all parameters in scenario and set up DoE calculated parameters
   for ( size_t i = 0; i < varSpace.numberOfContPrms(); ++i )
   {
      newCase->addParameter( varSpace.continuousParameter( i )->createNewParameterFromDouble( sumCntArray[ i ] ) );
   }
   for ( size_t i = 0; i < varSpace.numberOfCategPrms(); ++i )
   {
      newCase->addParameter( varSpace.categoricalParameter( i )->createNewParameterFromUnsignedInt( sumCatArray[ i ] ) );
   }
   
   expSet.push_back( newCase.release() );
}

}
