//                                                                      
// Copyright (C) 2017-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <iostream>
#include <vector>
#include <string>

// Mock
#include "MockSnapshot.h"
#include "MockFormation.h"
#include "MockGRPropertyManager.h"

// 3rdparty
#include "petsc.h"
#include <gtest/gtest.h>

//VALIDATION: For the generation of the unit test validation numbers please define VALIDATION
//#define VALIDATION
//IMPORTANT: run with as many cores as the original test to fet validation results please concatinate the results from the file >VALIDTATION_LOG_P<n>.txt<<
#ifdef VALIDATION
   #include <fstream>
   #include "CompoundLithologyArray.h"
#endif

using namespace DataModel;
using namespace DerivedProperties;

/// \brief (Unit-)Test of the GammaRay calculator in distributed mode
///  Tested are mock lithologies with different heat production values and the mock grid is alternating between defined and undefined area in the grid. 
TEST ( GammaRayFormationCalculatorDistributedTest, distributedComputation )
{

   // distributed test manager
   MockGRPropertyManager propertyManager(MockGRPropertyManager::DataAccessMode::DISTRIBUTED);
   const DataModel::AbstractProperty*  gammaRayProp = propertyManager.getProperty ( "GammaRay" );
   
   const shared_ptr<DataModel::AbstractSnapshot>  snapshot = make_shared<DataModel::MockSnapshot> ( 0.0 );
   const double currentTime = snapshot->getTime ();
   
   const shared_ptr<DataModel::AbstractFormation> formation = make_shared<MockFormation> ( "Formation","","",2);
   const FormationPropertyPtr gammaRay = propertyManager.getFormationProperty ( gammaRayProp, snapshot.get(), formation.get() );

#ifdef VALIDATION 
   const DataModel::AbstractProperty*  porosityProp = propertyManager.getProperty ( "Porosity" );
   const FormationPropertyPtr porosity = propertyManager.getFormationProperty ( porosityProp, snapshot.get(), formation.get() );
   const double undefinedValue = gammaRay->getUndefinedValue();
   const GeoPhysics::CompoundLithologyArray& lithologies = formation->getCompoundLithologyArray ();
#endif
    
   //prozessor rank and size for the distributed test   
   int rank,size;
   MPI_Comm_rank(PETSC_COMM_WORLD,&rank);
   MPI_Comm_size(PETSC_COMM_WORLD,&size);
   
   // This are the expected numbers [rank][i][j][k]
   std::vector< std::vector< std::vector< std::vector< double> > > > expected_results
            ( 2 , std::vector< std::vector< std::vector< double> > >( 6, std::vector< std::vector< double> > ( 10, std::vector<double>( 3 ) ) ) );
   expected_results [ 0 ][ 0 ][ 0 ][ 0 ] = 97.709090909090903665;
   expected_results [ 0 ][ 0 ][ 0 ][ 1 ] = 97.858181818181819267;
   expected_results [ 0 ][ 0 ][ 0 ][ 2 ] = 98.007272727272720658;
   expected_results [ 0 ][ 0 ][ 1 ][ 0 ] = 99999;
   expected_results [ 0 ][ 0 ][ 1 ][ 1 ] = 99999;
   expected_results [ 0 ][ 0 ][ 1 ][ 2 ] = 99999;
   expected_results [ 0 ][ 0 ][ 2 ][ 0 ] = 94.429090909090902528;
   expected_results [ 0 ][ 0 ][ 2 ][ 1 ] = 94.578181818181803919;
   expected_results [ 0 ][ 0 ][ 2 ][ 2 ] = 94.727272727272733732;
   expected_results [ 0 ][ 0 ][ 3 ][ 0 ] = 99999;
   expected_results [ 0 ][ 0 ][ 3 ][ 1 ] = 99999;
   expected_results [ 0 ][ 0 ][ 3 ][ 2 ] = 99999;
   expected_results [ 0 ][ 0 ][ 4 ][ 0 ] = 91.149090909090915602;
   expected_results [ 0 ][ 0 ][ 4 ][ 1 ] = 91.298181818181816993;
   expected_results [ 0 ][ 0 ][ 4 ][ 2 ] = 91.447272727272718384;
   expected_results [ 0 ][ 0 ][ 5 ][ 0 ] = 99999;
   expected_results [ 0 ][ 0 ][ 5 ][ 1 ] = 99999;
   expected_results [ 0 ][ 0 ][ 5 ][ 2 ] = 99999;
   expected_results [ 0 ][ 0 ][ 6 ][ 0 ] = 87.869090909090900254;
   expected_results [ 0 ][ 0 ][ 6 ][ 1 ] = 88.018181818181815856;
   expected_results [ 0 ][ 0 ][ 6 ][ 2 ] = 88.167272727272717248;
   expected_results [ 0 ][ 0 ][ 7 ][ 0 ] = 99999;
   expected_results [ 0 ][ 0 ][ 7 ][ 1 ] = 99999;
   expected_results [ 0 ][ 0 ][ 7 ][ 2 ] = 99999;
   expected_results [ 0 ][ 0 ][ 8 ][ 0 ] = 84.589090909090913328;
   expected_results [ 0 ][ 0 ][ 8 ][ 1 ] = 84.73818181818181472;
   expected_results [ 0 ][ 0 ][ 8 ][ 2 ] = 84.887272727272716111;
   expected_results [ 0 ][ 0 ][ 9 ][ 0 ] = 99999;
   expected_results [ 0 ][ 0 ][ 9 ][ 1 ] = 99999;
   expected_results [ 0 ][ 0 ][ 9 ][ 2 ] = 99999;
   expected_results [ 0 ][ 1 ][ 0 ][ 0 ] = 99999;
   expected_results [ 0 ][ 1 ][ 0 ][ 1 ] = 99999;
   expected_results [ 0 ][ 1 ][ 0 ][ 2 ] = 99999;
   expected_results [ 0 ][ 1 ][ 1 ][ 0 ] = 79.669090909090911623;
   expected_results [ 0 ][ 1 ][ 1 ][ 1 ] = 79.818181818181798803;
   expected_results [ 0 ][ 1 ][ 1 ][ 2 ] = 79.967272727272728616;
   expected_results [ 0 ][ 1 ][ 2 ][ 0 ] = 99999;
   expected_results [ 0 ][ 1 ][ 2 ][ 1 ] = 99999;
   expected_results [ 0 ][ 1 ][ 2 ][ 2 ] = 99999;
   expected_results [ 0 ][ 1 ][ 3 ][ 0 ] = 76.389090909090910486;
   expected_results [ 0 ][ 1 ][ 3 ][ 1 ] = 76.538181818181811877;
   expected_results [ 0 ][ 1 ][ 3 ][ 2 ] = 76.687272727272713269;
   expected_results [ 0 ][ 1 ][ 4 ][ 0 ] = 99999;
   expected_results [ 0 ][ 1 ][ 4 ][ 1 ] = 99999;
   expected_results [ 0 ][ 1 ][ 4 ][ 2 ] = 99999;
   expected_results [ 0 ][ 1 ][ 5 ][ 0 ] = 73.109090909090895138;
   expected_results [ 0 ][ 1 ][ 5 ][ 1 ] = 73.25818181818181074;
   expected_results [ 0 ][ 1 ][ 5 ][ 2 ] = 73.407272727272712132;
   expected_results [ 0 ][ 1 ][ 6 ][ 0 ] = 99999;
   expected_results [ 0 ][ 1 ][ 6 ][ 1 ] = 99999;
   expected_results [ 0 ][ 1 ][ 6 ][ 2 ] = 99999;
   expected_results [ 0 ][ 1 ][ 7 ][ 0 ] = 69.829090909090908212;
   expected_results [ 0 ][ 1 ][ 7 ][ 1 ] = 69.978181818181809604;
   expected_results [ 0 ][ 1 ][ 7 ][ 2 ] = 70.127272727272725206;
   expected_results [ 0 ][ 1 ][ 8 ][ 0 ] = 99999;
   expected_results [ 0 ][ 1 ][ 8 ][ 1 ] = 99999;
   expected_results [ 0 ][ 1 ][ 8 ][ 2 ] = 99999;
   expected_results [ 0 ][ 1 ][ 9 ][ 0 ] = 66.549090909090907076;
   expected_results [ 0 ][ 1 ][ 9 ][ 1 ] = 66.698181818181808467;
   expected_results [ 0 ][ 1 ][ 9 ][ 2 ] = 66.847272727272724069;
   expected_results [ 0 ][ 2 ][ 0 ][ 0 ] = 64.909090909090906507;
   expected_results [ 0 ][ 2 ][ 0 ][ 1 ] = 65.058181818181807898;
   expected_results [ 0 ][ 2 ][ 0 ][ 2 ] = 65.2072727272727235;
   expected_results [ 0 ][ 2 ][ 1 ][ 0 ] = 99999;
   expected_results [ 0 ][ 2 ][ 1 ][ 1 ] = 99999;
   expected_results [ 0 ][ 2 ][ 1 ][ 2 ] = 99999;
   expected_results [ 0 ][ 2 ][ 2 ][ 0 ] = 61.629090909090898265;
   expected_results [ 0 ][ 2 ][ 2 ][ 1 ] = 61.778181818181806761;
   expected_results [ 0 ][ 2 ][ 2 ][ 2 ] = 61.927272727272722364;
   expected_results [ 0 ][ 2 ][ 3 ][ 0 ] = 99999;
   expected_results [ 0 ][ 2 ][ 3 ][ 1 ] = 99999;
   expected_results [ 0 ][ 2 ][ 3 ][ 2 ] = 99999;
   expected_results [ 0 ][ 2 ][ 4 ][ 0 ] = 58.349090909090904233;
   expected_results [ 0 ][ 2 ][ 4 ][ 1 ] = 58.49818181818181273;
   expected_results [ 0 ][ 2 ][ 4 ][ 2 ] = 58.647272727272721227;
   expected_results [ 0 ][ 2 ][ 5 ][ 0 ] = 99999;
   expected_results [ 0 ][ 2 ][ 5 ][ 1 ] = 99999;
   expected_results [ 0 ][ 2 ][ 5 ][ 2 ] = 99999;
   expected_results [ 0 ][ 2 ][ 6 ][ 0 ] = 55.069090909090903097;
   expected_results [ 0 ][ 2 ][ 6 ][ 1 ] = 55.218181818181811593;
   expected_results [ 0 ][ 2 ][ 6 ][ 2 ] = 55.367272727272712984;
   expected_results [ 0 ][ 2 ][ 7 ][ 0 ] = 99999;
   expected_results [ 0 ][ 2 ][ 7 ][ 1 ] = 99999;
   expected_results [ 0 ][ 2 ][ 7 ][ 2 ] = 99999;
   expected_results [ 0 ][ 2 ][ 8 ][ 0 ] = 51.789090909090909065;
   expected_results [ 0 ][ 2 ][ 8 ][ 1 ] = 51.938181818181810456;
   expected_results [ 0 ][ 2 ][ 8 ][ 2 ] = 52.087272727272726058;
   expected_results [ 0 ][ 2 ][ 9 ][ 0 ] = 99999;
   expected_results [ 0 ][ 2 ][ 9 ][ 1 ] = 99999;
   expected_results [ 0 ][ 2 ][ 9 ][ 2 ] = 99999;
   expected_results [ 0 ][ 3 ][ 0 ][ 0 ] = 99999;
   expected_results [ 0 ][ 3 ][ 0 ][ 1 ] = 99999;
   expected_results [ 0 ][ 3 ][ 0 ][ 2 ] = 99999;
   expected_results [ 0 ][ 3 ][ 1 ][ 0 ] = 46.869090909090900254;
   expected_results [ 0 ][ 3 ][ 1 ][ 1 ] = 47.018181818181815856;
   expected_results [ 0 ][ 3 ][ 1 ][ 2 ] = 47.167272727272717248;
   expected_results [ 0 ][ 3 ][ 2 ][ 0 ] = 99999;
   expected_results [ 0 ][ 3 ][ 2 ][ 1 ] = 99999;
   expected_results [ 0 ][ 3 ][ 2 ][ 2 ] = 99999;
   expected_results [ 0 ][ 3 ][ 3 ][ 0 ] = 43.589090909090899117;
   expected_results [ 0 ][ 3 ][ 3 ][ 1 ] = 43.738181818181821825;
   expected_results [ 0 ][ 3 ][ 3 ][ 2 ] = 43.887272727272723216;
   expected_results [ 0 ][ 3 ][ 4 ][ 0 ] = 99999;
   expected_results [ 0 ][ 3 ][ 4 ][ 1 ] = 99999;
   expected_results [ 0 ][ 3 ][ 4 ][ 2 ] = 99999;
   expected_results [ 0 ][ 3 ][ 5 ][ 0 ] = 40.309090909090897981;
   expected_results [ 0 ][ 3 ][ 5 ][ 1 ] = 40.458181818181813583;
   expected_results [ 0 ][ 3 ][ 5 ][ 2 ] = 40.607272727272722079;
   expected_results [ 0 ][ 3 ][ 6 ][ 0 ] = 99999;
   expected_results [ 0 ][ 3 ][ 6 ][ 1 ] = 99999;
   expected_results [ 0 ][ 3 ][ 6 ][ 2 ] = 99999;
   expected_results [ 0 ][ 3 ][ 7 ][ 0 ] = 37.029090909090903949;
   expected_results [ 0 ][ 3 ][ 7 ][ 1 ] = 37.178181818181819551;
   expected_results [ 0 ][ 3 ][ 7 ][ 2 ] = 37.327272727272720942;
   expected_results [ 0 ][ 3 ][ 8 ][ 0 ] = 99999;
   expected_results [ 0 ][ 3 ][ 8 ][ 1 ] = 99999;
   expected_results [ 0 ][ 3 ][ 8 ][ 2 ] = 99999;
   expected_results [ 0 ][ 3 ][ 9 ][ 0 ] = 33.749090909090902812;
   expected_results [ 0 ][ 3 ][ 9 ][ 1 ] = 33.898181818181797098;
   expected_results [ 0 ][ 3 ][ 9 ][ 2 ] = 34.0472727272727127;
   expected_results [ 0 ][ 4 ][ 0 ][ 0 ] = 32.109090909090916455;
   expected_results [ 0 ][ 4 ][ 0 ][ 1 ] = 32.258181818181817846;
   expected_results [ 0 ][ 4 ][ 0 ][ 2 ] = 32.407272727272712132;
   expected_results [ 0 ][ 4 ][ 1 ][ 0 ] = 99999;
   expected_results [ 0 ][ 4 ][ 1 ][ 1 ] = 99999;
   expected_results [ 0 ][ 4 ][ 1 ][ 2 ] = 99999;
   expected_results [ 0 ][ 4 ][ 2 ][ 0 ] = 28.829090909090901107;
   expected_results [ 0 ][ 4 ][ 2 ][ 1 ] = 28.978181818181820262;
   expected_results [ 0 ][ 4 ][ 2 ][ 2 ] = 29.127272727272725206;
   expected_results [ 0 ][ 4 ][ 3 ][ 0 ] = 99999;
   expected_results [ 0 ][ 4 ][ 3 ][ 1 ] = 99999;
   expected_results [ 0 ][ 4 ][ 3 ][ 2 ] = 99999;
   expected_results [ 0 ][ 4 ][ 4 ][ 0 ] = 25.549090909090892865;
   expected_results [ 0 ][ 4 ][ 4 ][ 1 ] = 25.698181818181819125;
   expected_results [ 0 ][ 4 ][ 4 ][ 2 ] = 25.847272727272724069;
   expected_results [ 0 ][ 4 ][ 5 ][ 0 ] = 99999;
   expected_results [ 0 ][ 4 ][ 5 ][ 1 ] = 99999;
   expected_results [ 0 ][ 4 ][ 5 ][ 2 ] = 99999;
   expected_results [ 0 ][ 4 ][ 6 ][ 0 ] = 22.269090909090905939;
   expected_results [ 0 ][ 4 ][ 6 ][ 1 ] = 22.418181818181821541;
   expected_results [ 0 ][ 4 ][ 6 ][ 2 ] = 22.567272727272722932;
   expected_results [ 0 ][ 4 ][ 7 ][ 0 ] = 99999;
   expected_results [ 0 ][ 4 ][ 7 ][ 1 ] = 99999;
   expected_results [ 0 ][ 4 ][ 7 ][ 2 ] = 99999;
   expected_results [ 0 ][ 4 ][ 8 ][ 0 ] = 18.989090909090908355;
   expected_results [ 0 ][ 4 ][ 8 ][ 1 ] = 19.138181818181820404;
   expected_results [ 0 ][ 4 ][ 8 ][ 2 ] = 19.28727272727271469;
   expected_results [ 0 ][ 4 ][ 9 ][ 0 ] = 99999;
   expected_results [ 0 ][ 4 ][ 9 ][ 1 ] = 99999;
   expected_results [ 0 ][ 4 ][ 9 ][ 2 ] = 99999;
   expected_results [ 0 ][ 5 ][ 0 ][ 0 ] = 99999;
   expected_results [ 0 ][ 5 ][ 0 ][ 1 ] = 99999;
   expected_results [ 0 ][ 5 ][ 0 ][ 2 ] = 99999;
   expected_results [ 0 ][ 5 ][ 1 ][ 0 ] = 14.06909090909090132;
   expected_results [ 0 ][ 5 ][ 1 ][ 1 ] = 14.218181818181818699;
   expected_results [ 0 ][ 5 ][ 1 ][ 2 ] = 14.367272727272732524;
   expected_results [ 0 ][ 5 ][ 2 ][ 0 ] = 99999;
   expected_results [ 0 ][ 5 ][ 2 ][ 1 ] = 99999;
   expected_results [ 0 ][ 5 ][ 2 ][ 2 ] = 99999;
   expected_results [ 0 ][ 5 ][ 3 ][ 0 ] = 10.789090909090914394;
   expected_results [ 0 ][ 5 ][ 3 ][ 1 ] = 10.938181818181806904;
   expected_results [ 0 ][ 5 ][ 3 ][ 2 ] = 11.087272727272724282;
   expected_results [ 0 ][ 5 ][ 4 ][ 0 ] = 99999;
   expected_results [ 0 ][ 5 ][ 4 ][ 1 ] = 99999;
   expected_results [ 0 ][ 5 ][ 4 ][ 2 ] = 99999;
   expected_results [ 0 ][ 5 ][ 5 ][ 0 ] = 7.5090909090909141455;
   expected_results [ 0 ][ 5 ][ 5 ][ 1 ] = 7.6581818181818190894;
   expected_results [ 0 ][ 5 ][ 5 ][ 2 ] = 7.807272727272712487;
   expected_results [ 0 ][ 5 ][ 6 ][ 0 ] = 99999;
   expected_results [ 0 ][ 5 ][ 6 ][ 1 ] = 99999;
   expected_results [ 0 ][ 5 ][ 6 ][ 2 ] = 99999;
   expected_results [ 0 ][ 5 ][ 7 ][ 0 ] = 4.2290909090909041268;
   expected_results [ 0 ][ 5 ][ 7 ][ 1 ] = 4.3781818181818188407;
   expected_results [ 0 ][ 5 ][ 7 ][ 2 ] = 4.5272727272727237846;
   expected_results [ 0 ][ 5 ][ 8 ][ 0 ] = 99999;
   expected_results [ 0 ][ 5 ][ 8 ][ 1 ] = 99999;
   expected_results [ 0 ][ 5 ][ 8 ][ 2 ] = 99999;
   expected_results [ 0 ][ 5 ][ 9 ][ 0 ] = 0.94909090909089366406;
   expected_results [ 0 ][ 5 ][ 9 ][ 1 ] = 1.0981818181818201463;
   expected_results [ 0 ][ 5 ][ 9 ][ 2 ] = 1.2472727272727248682;
   expected_results [ 1 ][ 0 ][ 0 ][ 0 ] = 97.709090909090903665;
   expected_results [ 1 ][ 0 ][ 0 ][ 1 ] = 97.858181818181819267;
   expected_results [ 1 ][ 0 ][ 0 ][ 2 ] = 98.007272727272720658;
   expected_results [ 1 ][ 0 ][ 1 ][ 0 ] = 99999;
   expected_results [ 1 ][ 0 ][ 1 ][ 1 ] = 99999;
   expected_results [ 1 ][ 0 ][ 1 ][ 2 ] = 99999;
   expected_results [ 1 ][ 0 ][ 2 ][ 0 ] = 94.429090909090902528;
   expected_results [ 1 ][ 0 ][ 2 ][ 1 ] = 94.578181818181803919;
   expected_results [ 1 ][ 0 ][ 2 ][ 2 ] = 94.727272727272733732;
   expected_results [ 1 ][ 0 ][ 3 ][ 0 ] = 99999;
   expected_results [ 1 ][ 0 ][ 3 ][ 1 ] = 99999;
   expected_results [ 1 ][ 0 ][ 3 ][ 2 ] = 99999;
   expected_results [ 1 ][ 0 ][ 4 ][ 0 ] = 91.149090909090915602;
   expected_results [ 1 ][ 0 ][ 4 ][ 1 ] = 91.298181818181816993;
   expected_results [ 1 ][ 0 ][ 4 ][ 2 ] = 91.447272727272718384;
   expected_results [ 1 ][ 0 ][ 5 ][ 0 ] = 99999;
   expected_results [ 1 ][ 0 ][ 5 ][ 1 ] = 99999;
   expected_results [ 1 ][ 0 ][ 5 ][ 2 ] = 99999;
   expected_results [ 1 ][ 0 ][ 6 ][ 0 ] = 87.869090909090900254;
   expected_results [ 1 ][ 0 ][ 6 ][ 1 ] = 88.018181818181815856;
   expected_results [ 1 ][ 0 ][ 6 ][ 2 ] = 88.167272727272717248;
   expected_results [ 1 ][ 0 ][ 7 ][ 0 ] = 99999;
   expected_results [ 1 ][ 0 ][ 7 ][ 1 ] = 99999;
   expected_results [ 1 ][ 0 ][ 7 ][ 2 ] = 99999;
   expected_results [ 1 ][ 0 ][ 8 ][ 0 ] = 84.589090909090913328;
   expected_results [ 1 ][ 0 ][ 8 ][ 1 ] = 84.73818181818181472;
   expected_results [ 1 ][ 0 ][ 8 ][ 2 ] = 84.887272727272716111;
   expected_results [ 1 ][ 0 ][ 9 ][ 0 ] = 99999;
   expected_results [ 1 ][ 0 ][ 9 ][ 1 ] = 99999;
   expected_results [ 1 ][ 0 ][ 9 ][ 2 ] = 99999;
   expected_results [ 1 ][ 1 ][ 0 ][ 0 ] = 99999;
   expected_results [ 1 ][ 1 ][ 0 ][ 1 ] = 99999;
   expected_results [ 1 ][ 1 ][ 0 ][ 2 ] = 99999;
   expected_results [ 1 ][ 1 ][ 1 ][ 0 ] = 79.669090909090911623;
   expected_results [ 1 ][ 1 ][ 1 ][ 1 ] = 79.818181818181798803;
   expected_results [ 1 ][ 1 ][ 1 ][ 2 ] = 79.967272727272728616;
   expected_results [ 1 ][ 1 ][ 2 ][ 0 ] = 99999;
   expected_results [ 1 ][ 1 ][ 2 ][ 1 ] = 99999;
   expected_results [ 1 ][ 1 ][ 2 ][ 2 ] = 99999;
   expected_results [ 1 ][ 1 ][ 3 ][ 0 ] = 76.389090909090910486;
   expected_results [ 1 ][ 1 ][ 3 ][ 1 ] = 76.538181818181811877;
   expected_results [ 1 ][ 1 ][ 3 ][ 2 ] = 76.687272727272713269;
   expected_results [ 1 ][ 1 ][ 4 ][ 0 ] = 99999;
   expected_results [ 1 ][ 1 ][ 4 ][ 1 ] = 99999;
   expected_results [ 1 ][ 1 ][ 4 ][ 2 ] = 99999;
   expected_results [ 1 ][ 1 ][ 5 ][ 0 ] = 73.109090909090895138;
   expected_results [ 1 ][ 1 ][ 5 ][ 1 ] = 73.25818181818181074;
   expected_results [ 1 ][ 1 ][ 5 ][ 2 ] = 73.407272727272712132;
   expected_results [ 1 ][ 1 ][ 6 ][ 0 ] = 99999;
   expected_results [ 1 ][ 1 ][ 6 ][ 1 ] = 99999;
   expected_results [ 1 ][ 1 ][ 6 ][ 2 ] = 99999;
   expected_results [ 1 ][ 1 ][ 7 ][ 0 ] = 69.829090909090908212;
   expected_results [ 1 ][ 1 ][ 7 ][ 1 ] = 69.978181818181809604;
   expected_results [ 1 ][ 1 ][ 7 ][ 2 ] = 70.127272727272725206;
   expected_results [ 1 ][ 1 ][ 8 ][ 0 ] = 99999;
   expected_results [ 1 ][ 1 ][ 8 ][ 1 ] = 99999;
   expected_results [ 1 ][ 1 ][ 8 ][ 2 ] = 99999;
   expected_results [ 1 ][ 1 ][ 9 ][ 0 ] = 66.549090909090907076;
   expected_results [ 1 ][ 1 ][ 9 ][ 1 ] = 66.698181818181808467;
   expected_results [ 1 ][ 1 ][ 9 ][ 2 ] = 66.847272727272724069;
   expected_results [ 1 ][ 2 ][ 0 ][ 0 ] = 64.909090909090906507;
   expected_results [ 1 ][ 2 ][ 0 ][ 1 ] = 65.058181818181807898;
   expected_results [ 1 ][ 2 ][ 0 ][ 2 ] = 65.2072727272727235;
   expected_results [ 1 ][ 2 ][ 1 ][ 0 ] = 99999;
   expected_results [ 1 ][ 2 ][ 1 ][ 1 ] = 99999;
   expected_results [ 1 ][ 2 ][ 1 ][ 2 ] = 99999;
   expected_results [ 1 ][ 2 ][ 2 ][ 0 ] = 61.629090909090898265;
   expected_results [ 1 ][ 2 ][ 2 ][ 1 ] = 61.778181818181806761;
   expected_results [ 1 ][ 2 ][ 2 ][ 2 ] = 61.927272727272722364;
   expected_results [ 1 ][ 2 ][ 3 ][ 0 ] = 99999;
   expected_results [ 1 ][ 2 ][ 3 ][ 1 ] = 99999;
   expected_results [ 1 ][ 2 ][ 3 ][ 2 ] = 99999;
   expected_results [ 1 ][ 2 ][ 4 ][ 0 ] = 58.349090909090904233;
   expected_results [ 1 ][ 2 ][ 4 ][ 1 ] = 58.49818181818181273;
   expected_results [ 1 ][ 2 ][ 4 ][ 2 ] = 58.647272727272721227;
   expected_results [ 1 ][ 2 ][ 5 ][ 0 ] = 99999;
   expected_results [ 1 ][ 2 ][ 5 ][ 1 ] = 99999;
   expected_results [ 1 ][ 2 ][ 5 ][ 2 ] = 99999;
   expected_results [ 1 ][ 2 ][ 6 ][ 0 ] = 55.069090909090903097;
   expected_results [ 1 ][ 2 ][ 6 ][ 1 ] = 55.218181818181811593;
   expected_results [ 1 ][ 2 ][ 6 ][ 2 ] = 55.367272727272712984;
   expected_results [ 1 ][ 2 ][ 7 ][ 0 ] = 99999;
   expected_results [ 1 ][ 2 ][ 7 ][ 1 ] = 99999;
   expected_results [ 1 ][ 2 ][ 7 ][ 2 ] = 99999;
   expected_results [ 1 ][ 2 ][ 8 ][ 0 ] = 51.789090909090909065;
   expected_results [ 1 ][ 2 ][ 8 ][ 1 ] = 51.938181818181810456;
   expected_results [ 1 ][ 2 ][ 8 ][ 2 ] = 52.087272727272726058;
   expected_results [ 1 ][ 2 ][ 9 ][ 0 ] = 99999;
   expected_results [ 1 ][ 2 ][ 9 ][ 1 ] = 99999;
   expected_results [ 1 ][ 2 ][ 9 ][ 2 ] = 99999;
   expected_results [ 1 ][ 3 ][ 0 ][ 0 ] = 99999;
   expected_results [ 1 ][ 3 ][ 0 ][ 1 ] = 99999;
   expected_results [ 1 ][ 3 ][ 0 ][ 2 ] = 99999;
   expected_results [ 1 ][ 3 ][ 1 ][ 0 ] = 46.869090909090900254;
   expected_results [ 1 ][ 3 ][ 1 ][ 1 ] = 47.018181818181815856;
   expected_results [ 1 ][ 3 ][ 1 ][ 2 ] = 47.167272727272717248;
   expected_results [ 1 ][ 3 ][ 2 ][ 0 ] = 99999;
   expected_results [ 1 ][ 3 ][ 2 ][ 1 ] = 99999;
   expected_results [ 1 ][ 3 ][ 2 ][ 2 ] = 99999;
   expected_results [ 1 ][ 3 ][ 3 ][ 0 ] = 43.589090909090899117;
   expected_results [ 1 ][ 3 ][ 3 ][ 1 ] = 43.738181818181821825;
   expected_results [ 1 ][ 3 ][ 3 ][ 2 ] = 43.887272727272723216;
   expected_results [ 1 ][ 3 ][ 4 ][ 0 ] = 99999;
   expected_results [ 1 ][ 3 ][ 4 ][ 1 ] = 99999;
   expected_results [ 1 ][ 3 ][ 4 ][ 2 ] = 99999;
   expected_results [ 1 ][ 3 ][ 5 ][ 0 ] = 40.309090909090897981;
   expected_results [ 1 ][ 3 ][ 5 ][ 1 ] = 40.458181818181813583;
   expected_results [ 1 ][ 3 ][ 5 ][ 2 ] = 40.607272727272722079;
   expected_results [ 1 ][ 3 ][ 6 ][ 0 ] = 99999;
   expected_results [ 1 ][ 3 ][ 6 ][ 1 ] = 99999;
   expected_results [ 1 ][ 3 ][ 6 ][ 2 ] = 99999;
   expected_results [ 1 ][ 3 ][ 7 ][ 0 ] = 37.029090909090903949;
   expected_results [ 1 ][ 3 ][ 7 ][ 1 ] = 37.178181818181819551;
   expected_results [ 1 ][ 3 ][ 7 ][ 2 ] = 37.327272727272720942;
   expected_results [ 1 ][ 3 ][ 8 ][ 0 ] = 99999;
   expected_results [ 1 ][ 3 ][ 8 ][ 1 ] = 99999;
   expected_results [ 1 ][ 3 ][ 8 ][ 2 ] = 99999;
   expected_results [ 1 ][ 3 ][ 9 ][ 0 ] = 33.749090909090902812;
   expected_results [ 1 ][ 3 ][ 9 ][ 1 ] = 33.898181818181797098;
   expected_results [ 1 ][ 3 ][ 9 ][ 2 ] = 34.0472727272727127;
   expected_results [ 1 ][ 4 ][ 0 ][ 0 ] = 32.109090909090916455;
   expected_results [ 1 ][ 4 ][ 0 ][ 1 ] = 32.258181818181817846;
   expected_results [ 1 ][ 4 ][ 0 ][ 2 ] = 32.407272727272712132;
   expected_results [ 1 ][ 4 ][ 1 ][ 0 ] = 99999;
   expected_results [ 1 ][ 4 ][ 1 ][ 1 ] = 99999;
   expected_results [ 1 ][ 4 ][ 1 ][ 2 ] = 99999;
   expected_results [ 1 ][ 4 ][ 2 ][ 0 ] = 28.829090909090901107;
   expected_results [ 1 ][ 4 ][ 2 ][ 1 ] = 28.978181818181820262;
   expected_results [ 1 ][ 4 ][ 2 ][ 2 ] = 29.127272727272725206;
   expected_results [ 1 ][ 4 ][ 3 ][ 0 ] = 99999;
   expected_results [ 1 ][ 4 ][ 3 ][ 1 ] = 99999;
   expected_results [ 1 ][ 4 ][ 3 ][ 2 ] = 99999;
   expected_results [ 1 ][ 4 ][ 4 ][ 0 ] = 25.549090909090892865;
   expected_results [ 1 ][ 4 ][ 4 ][ 1 ] = 25.698181818181819125;
   expected_results [ 1 ][ 4 ][ 4 ][ 2 ] = 25.847272727272724069;
   expected_results [ 1 ][ 4 ][ 5 ][ 0 ] = 99999;
   expected_results [ 1 ][ 4 ][ 5 ][ 1 ] = 99999;
   expected_results [ 1 ][ 4 ][ 5 ][ 2 ] = 99999;
   expected_results [ 1 ][ 4 ][ 6 ][ 0 ] = 22.269090909090905939;
   expected_results [ 1 ][ 4 ][ 6 ][ 1 ] = 22.418181818181821541;
   expected_results [ 1 ][ 4 ][ 6 ][ 2 ] = 22.567272727272722932;
   expected_results [ 1 ][ 4 ][ 7 ][ 0 ] = 99999;
   expected_results [ 1 ][ 4 ][ 7 ][ 1 ] = 99999;
   expected_results [ 1 ][ 4 ][ 7 ][ 2 ] = 99999;
   expected_results [ 1 ][ 4 ][ 8 ][ 0 ] = 18.989090909090908355;
   expected_results [ 1 ][ 4 ][ 8 ][ 1 ] = 19.138181818181820404;
   expected_results [ 1 ][ 4 ][ 8 ][ 2 ] = 19.28727272727271469;
   expected_results [ 1 ][ 4 ][ 9 ][ 0 ] = 99999;
   expected_results [ 1 ][ 4 ][ 9 ][ 1 ] = 99999;
   expected_results [ 1 ][ 4 ][ 9 ][ 2 ] = 99999;
   expected_results [ 1 ][ 5 ][ 0 ][ 0 ] = 99999;
   expected_results [ 1 ][ 5 ][ 0 ][ 1 ] = 99999;
   expected_results [ 1 ][ 5 ][ 0 ][ 2 ] = 99999;
   expected_results [ 1 ][ 5 ][ 1 ][ 0 ] = 14.06909090909090132;
   expected_results [ 1 ][ 5 ][ 1 ][ 1 ] = 14.218181818181818699;
   expected_results [ 1 ][ 5 ][ 1 ][ 2 ] = 14.367272727272732524;
   expected_results [ 1 ][ 5 ][ 2 ][ 0 ] = 99999;
   expected_results [ 1 ][ 5 ][ 2 ][ 1 ] = 99999;
   expected_results [ 1 ][ 5 ][ 2 ][ 2 ] = 99999;
   expected_results [ 1 ][ 5 ][ 3 ][ 0 ] = 10.789090909090914394;
   expected_results [ 1 ][ 5 ][ 3 ][ 1 ] = 10.938181818181806904;
   expected_results [ 1 ][ 5 ][ 3 ][ 2 ] = 11.087272727272724282;
   expected_results [ 1 ][ 5 ][ 4 ][ 0 ] = 99999;
   expected_results [ 1 ][ 5 ][ 4 ][ 1 ] = 99999;
   expected_results [ 1 ][ 5 ][ 4 ][ 2 ] = 99999;
   expected_results [ 1 ][ 5 ][ 5 ][ 0 ] = 7.5090909090909141455;
   expected_results [ 1 ][ 5 ][ 5 ][ 1 ] = 7.6581818181818190894;
   expected_results [ 1 ][ 5 ][ 5 ][ 2 ] = 7.807272727272712487;
   expected_results [ 1 ][ 5 ][ 6 ][ 0 ] = 99999;
   expected_results [ 1 ][ 5 ][ 6 ][ 1 ] = 99999;
   expected_results [ 1 ][ 5 ][ 6 ][ 2 ] = 99999;
   expected_results [ 1 ][ 5 ][ 7 ][ 0 ] = 4.2290909090909041268;
   expected_results [ 1 ][ 5 ][ 7 ][ 1 ] = 4.3781818181818188407;
   expected_results [ 1 ][ 5 ][ 7 ][ 2 ] = 4.5272727272727237846;
   expected_results [ 1 ][ 5 ][ 8 ][ 0 ] = 99999;
   expected_results [ 1 ][ 5 ][ 8 ][ 1 ] = 99999;
   expected_results [ 1 ][ 5 ][ 8 ][ 2 ] = 99999;
   expected_results [ 1 ][ 5 ][ 9 ][ 0 ] = 0.94909090909089366406;
   expected_results [ 1 ][ 5 ][ 9 ][ 1 ] = 1.0981818181818201463;
   expected_results [ 1 ][ 5 ][ 9 ][ 2 ] = 1.2472727272727248682;

#ifdef VALIDATION
   //These number are only needed for the calculation of the validation numbers
   const double gammaRayScaleFactor=0.0158;
   const double gammaRayOffset=0.8; 

   //Generate validation output
   std::ofstream myfile;
   myfile.open ("VALIDTATION_LOG_P" + std::to_string(rank) + ".log");
   myfile<<"These are the validation data:"<<std::endl;
   myfile<<"std::vector< std::vector< std::vector< std::vector< double> > > > expected_results( "<<size<<" , std::vector< std::vector< std::vector< double> > >( "<<gammaRay->lastI ( true )+1 - gammaRay->firstI ( true )<<", std::vector< std::vector< double> > ( "<<gammaRay->lastJ ( true )+1-gammaRay->firstJ ( true )<<", std::vector<double>( " <<gammaRay->lengthK()<<" ) ) ) );"<<std::endl;
#endif

   for ( unsigned int i = gammaRay->firstI ( true ); i <= gammaRay->lastI ( true ); ++i ) {
      for ( unsigned int j = gammaRay->firstJ ( true ); j <= gammaRay->lastJ ( true ); ++j ) {
        
#ifdef VALIDATION
         const double solidRadiogenicHeatProduction = lithologies ( i, j, currentTime )-> heatproduction ();
#endif
         
         for ( unsigned int k = gammaRay->firstK (); k <= gammaRay->lastK (); ++k ) {
	   
#ifdef VALIDATION 
            //Generate validation output 
            if( (i+j) % 2 == 0){
               myfile<<"expected_results [ "<<rank<<" ][ "<<i - gammaRay->firstI ( true )<<" ][ "<<j - gammaRay->firstJ ( true )<<" ][ "<<k - gammaRay->firstK ()<<" ] = "<<std::setprecision(20)<<((((1 - (0.01 * porosity->getA ( i, j, k )) ) * solidRadiogenicHeatProduction) / gammaRayScaleFactor) + gammaRayOffset)<<";"<<std::endl;;
            }else{
               myfile<<"expected_results [ "<<rank<<" ][ "<<i - gammaRay->firstI ( true )<<" ][ "<<j - gammaRay->firstJ ( true )<<" ][ "<<k - gammaRay->firstK ()<<" ] = "<<std::setprecision(20)<<undefinedValue<<";"<<std::endl;;
            }
#else
            EXPECT_DOUBLE_EQ ( gammaRay->get ( i, j, k ), expected_results [rank] [i-gammaRay->firstI ( true )][j - gammaRay->firstJ ( true )][k - gammaRay->firstK ()] ); 
#endif

         }

      }

   }
   
#ifdef VALIDATION 
   myfile.close();
#endif

}

