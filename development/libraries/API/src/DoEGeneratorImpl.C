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
#include "ContinuousParameter.h"
#include "CategoricalParameter.h"
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

DoEGeneratorImpl::DoEGeneratorImpl( mbapi::Model & baseModel, DoEGenerator::DoEAlgorithm algo ) : 
                  m_baseModel( baseModel )
                , m_typeOfDoE( algo )
{
}

DoEGeneratorImpl::~DoEGeneratorImpl()
{
}

// Generate set of cases for DoE
// [in]  varPrmsSet list of variable parameters
// [out] expSet list of cases for DoE
// [in]  runsNum number of runs for DoE algorithms which support this parameter
// return ErrorHandler::NoError on success, error code otherwise
ErrorHandler::ReturnCode DoEGeneratorImpl::generateDoE( const VarSpace & varPrmsSet, std::vector<RunCase*> & expSet, size_t runsNum )
{
   const std::vector< bool > selectedPrms( varPrmsSet.size(), true );

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
         const SUMlib::HybridMC doe( selectedPrms, varPrmsSet.numberOfContPrms(), expSet.size(), runsNum );
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
         case BoxBehnken:           doe.reset( new SUMlib::BoxBehnken(   selectedPrms, varPrmsSet.numberOfContPrms()               ) ); break;
         case Tornado:              doe.reset( new SUMlib::Tornado(      selectedPrms, varPrmsSet.numberOfContPrms()               ) ); break;
         case PlackettBurman:       doe.reset( new SUMlib::ScreenDesign( selectedPrms, varPrmsSet.numberOfContPrms(), false, false ) ); break;
         case PlackettBurmanMirror: doe.reset( new SUMlib::ScreenDesign( selectedPrms, varPrmsSet.numberOfContPrms(), false, true  ) ); break;
         case LatinHypercube:       doe.reset( new SUMlib::OptimisedLHD( selectedPrms, varPrmsSet.numberOfContPrms(), runsNum      ) ); break;
         case FullFactorial:        doe.reset( new SUMlib::FactDesign(   selectedPrms, varPrmsSet.numberOfContPrms()               ) ); break;
         default:
            std::ostringstream oss;
            oss << "Unknown DoE algorithm: " << m_typeOfDoE;
            return ReportError( UndefinedValue, oss.str() );
         }
         doe->getCaseSet( pBounds, baseCase, replicate, sumCases );
      }

      for ( std::vector<SUMlib::Case>::const_iterator c = sumCases.begin(); c != sumCases.end(); ++c )
      {
         addCase( varPrmsSet, expSet, *c );
      }
   }
   catch ( const Exception & e )
   {
         std::ostringstream oss;
         oss << "SUMlib exception caught on DoE case generation: " << e.what();
         return ReportError( SUMLibException, oss.str() );
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
   const std::vector<ContinuousParameter*> cntPrms = varSpace.continuousParameters();
   
   std::vector<double> minCntPrms( cntPrms.size() );
   std::vector<double> maxCntPrms( cntPrms.size() );

   for ( size_t i = 0; i < cntPrms.size(); ++i )
   {
      minCntPrms[i] = cntPrms[i]->minValueAsDouble();
      maxCntPrms[i] = cntPrms[i]->maxValueAsDouble();
   }
   lowCs.setContinuousPart( minCntPrms );
   highCs.setContinuousPart( maxCntPrms );

   // process categorical parameters set
   const std::vector<CategoricalParameter*> catPrms = varSpace.categoricalParameters();
   
   // clean container for categorical values
   catIndices.clear();

   for ( size_t i = 0; i < catPrms.size(); ++i )
   {
      const std::vector<unsigned int> & valsSet = catPrms[i]->valuesAsUnsignedIntSortedSet();
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
   const std::vector<ContinuousParameter*> cntPrms = varSpace.continuousParameters( );

   std::vector<double> baseCntPrms( cntPrms.size( ) );

   for ( size_t i = 0; i < cntPrms.size( ); ++i )
   {
      baseCntPrms[ i ] = cntPrms[ i ]->baseValueAsDouble( );
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
   /// TODO: create here case with copy of base case model
   // std::auto_ptr<RunCaseImpl> newCase( new RunCaseImpl( m_baseModel ) );
   std::auto_ptr<RunCaseImpl> newCase( new RunCaseImpl() );

   std::vector<Parameter*>  & newCasePrms = newCase->parametersSet();

   const std::vector<double>       & sumCntArray = cs.continuousPart();
   const std::vector<unsigned int> & sumCatArray = cs.categoricalPart();

   // go over all parameters in scenario and set up DoE calculated parameters
   const std::vector<ContinuousParameter*>  cntPrms = varSpace.continuousParameters();
   const std::vector<CategoricalParameter*> catPrms = varSpace.categoricalParameters();

   for ( size_t i = 0; i < cntPrms.size(); ++i )
   {
      newCasePrms.push_back( cntPrms[ i ]->createNewParameterFromDouble( sumCntArray[ i ] ) );
   }
   for ( size_t i = 0; i < catPrms.size(); ++i )
   {
      newCasePrms.push_back( catPrms[ i ]->createNewParameterFromUnsignedInt( sumCatArray[ i ] ) );
   }
   
   expSet.push_back( newCase.release() );
}

}
