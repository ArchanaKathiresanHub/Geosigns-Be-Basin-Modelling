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
#include <gtest/gtest.h>

//VALIDATION: For the generation of the unit test validation numbers please define VALIDATION
//#define VALIDATION
#ifdef VALIDATION
   #include <fstream>
#endif

using namespace DataModel;
using namespace AbstractDerivedProperties;
using namespace std;

/// \brief (Unit-)Test of the GammaRay calculator.
///  Tested are mock lithologies with different heat production values and the mock grid is alternating between defined and undefined area in the grid. 
TEST ( GammaRayFormationCalculatorTest, computation )
{

   // Sequential test manager
   MockGRPropertyManager propertyManager(MockGRPropertyManager::DataAccessMode::SERIAL);

   const DataModel::AbstractProperty*  gammaRayProp = propertyManager.getProperty ( "GammaRay" );

   const shared_ptr<DataModel::AbstractSnapshot>  snapshot = make_shared<MockSnapshot>( 0.0 );
   const double currentTime = snapshot->getTime ();
   
   const shared_ptr<DataModel::AbstractFormation> formation = make_shared<MockFormation>( "Formation","","",2);
   const FormationPropertyPtr gammaRay = propertyManager.getFormationProperty ( gammaRayProp, snapshot.get(), formation.get() );
  
#ifdef VALIDATION
   const DataModel::AbstractProperty*  porosityProp = propertyManager.getProperty ( "Porosity" );
   const FormationPropertyPtr porosity = propertyManager.getFormationProperty ( porosityProp, snapshot.get(), formation.get() );
   const double undefinedValue = gammaRay->getUndefinedValue();
#endif

   // This are the expected numbers  [i][j][k]
   std::vector< std::vector< std::vector< double>> > expected_results(11, std::vector< std::vector< double> > ( 11, std::vector<double>( 3 ) ) );
   expected_results [ 0 ][ 0 ][ 0 ] = 98.460706235912837769;
   expected_results [ 0 ][ 0 ][ 1 ] = 98.534635612321565645;
   expected_results [ 0 ][ 0 ][ 2 ] = 98.6085649887302651;
   expected_results [ 0 ][ 1 ][ 0 ] = 99999;
   expected_results [ 0 ][ 1 ][ 1 ] = 99999;
   expected_results [ 0 ][ 1 ][ 2 ] = 99999;
   expected_results [ 0 ][ 2 ][ 0 ] = 96.834259954921108715;
   expected_results [ 0 ][ 2 ][ 1 ] = 96.90818933132982238;
   expected_results [ 0 ][ 2 ][ 2 ] = 96.982118707738536045;
   expected_results [ 0 ][ 3 ][ 0 ] = 99999;
   expected_results [ 0 ][ 3 ][ 1 ] = 99999;
   expected_results [ 0 ][ 3 ][ 2 ] = 99999;
   expected_results [ 0 ][ 4 ][ 0 ] = 95.20781367392936545;
   expected_results [ 0 ][ 4 ][ 1 ] = 95.281743050338079115;
   expected_results [ 0 ][ 4 ][ 2 ] = 95.35567242674679278;
   expected_results [ 0 ][ 5 ][ 0 ] = 99999;
   expected_results [ 0 ][ 5 ][ 1 ] = 99999;
   expected_results [ 0 ][ 5 ][ 2 ] = 99999;
   expected_results [ 0 ][ 6 ][ 0 ] = 93.581367392937650607;
   expected_results [ 0 ][ 6 ][ 1 ] = 93.65529676934633585;
   expected_results [ 0 ][ 6 ][ 2 ] = 93.729226145755063726;
   expected_results [ 0 ][ 7 ][ 0 ] = 99999;
   expected_results [ 0 ][ 7 ][ 1 ] = 99999;
   expected_results [ 0 ][ 7 ][ 2 ] = 99999;
   expected_results [ 0 ][ 8 ][ 0 ] = 91.954921111945893131;
   expected_results [ 0 ][ 8 ][ 1 ] = 92.028850488354621007;
   expected_results [ 0 ][ 8 ][ 2 ] = 92.102779864763334672;
   expected_results [ 0 ][ 9 ][ 0 ] = 99999;
   expected_results [ 0 ][ 9 ][ 1 ] = 99999;
   expected_results [ 0 ][ 9 ][ 2 ] = 99999;
   expected_results [ 0 ][ 10 ][ 0 ] = 90.328474830954164077;
   expected_results [ 0 ][ 10 ][ 1 ] = 90.402404207362877742;
   expected_results [ 0 ][ 10 ][ 2 ] = 90.476333583771591407;
   expected_results [ 1 ][ 0 ][ 0 ] = 99999;
   expected_results [ 1 ][ 0 ][ 1 ] = 99999;
   expected_results [ 1 ][ 0 ][ 2 ] = 99999;
   expected_results [ 1 ][ 1 ][ 0 ] = 88.702028549962420811;
   expected_results [ 1 ][ 1 ][ 1 ] = 88.775957926371148687;
   expected_results [ 1 ][ 1 ][ 2 ] = 88.849887302779848142;
   expected_results [ 1 ][ 2 ][ 0 ] = 99999;
   expected_results [ 1 ][ 2 ][ 1 ] = 99999;
   expected_results [ 1 ][ 2 ][ 2 ] = 99999;
   expected_results [ 1 ][ 3 ][ 0 ] = 87.075582268970691757;
   expected_results [ 1 ][ 3 ][ 1 ] = 87.149511645379405422;
   expected_results [ 1 ][ 3 ][ 2 ] = 87.223441021788119087;
   expected_results [ 1 ][ 4 ][ 0 ] = 99999;
   expected_results [ 1 ][ 4 ][ 1 ] = 99999;
   expected_results [ 1 ][ 4 ][ 2 ] = 99999;
   expected_results [ 1 ][ 5 ][ 0 ] = 85.449135987978962703;
   expected_results [ 1 ][ 5 ][ 1 ] = 85.523065364387662157;
   expected_results [ 1 ][ 5 ][ 2 ] = 85.596994740796390033;
   expected_results [ 1 ][ 6 ][ 0 ] = 99999;
   expected_results [ 1 ][ 6 ][ 1 ] = 99999;
   expected_results [ 1 ][ 6 ][ 2 ] = 99999;
   expected_results [ 1 ][ 7 ][ 0 ] = 83.822689706987233649;
   expected_results [ 1 ][ 7 ][ 1 ] = 83.896619083395933103;
   expected_results [ 1 ][ 7 ][ 2 ] = 83.970548459804660979;
   expected_results [ 1 ][ 8 ][ 0 ] = 99999;
   expected_results [ 1 ][ 8 ][ 1 ] = 99999;
   expected_results [ 1 ][ 8 ][ 2 ] = 99999;
   expected_results [ 1 ][ 9 ][ 0 ] = 82.196243425995490384;
   expected_results [ 1 ][ 9 ][ 1 ] = 82.270172802404204049;
   expected_results [ 1 ][ 9 ][ 2 ] = 82.344102178812917714;
   expected_results [ 1 ][ 10 ][ 0 ] = 99999;
   expected_results [ 1 ][ 10 ][ 1 ] = 99999;
   expected_results [ 1 ][ 10 ][ 2 ] = 99999;
   expected_results [ 2 ][ 0 ][ 0 ] = 80.569797145003732908;
   expected_results [ 2 ][ 0 ][ 1 ] = 80.643726521412474995;
   expected_results [ 2 ][ 0 ][ 2 ] = 80.717655897821174449;
   expected_results [ 2 ][ 1 ][ 0 ] = 99999;
   expected_results [ 2 ][ 1 ][ 1 ] = 99999;
   expected_results [ 2 ][ 1 ][ 2 ] = 99999;
   expected_results [ 2 ][ 2 ][ 0 ] = 78.943350864012003854;
   expected_results [ 2 ][ 2 ][ 1 ] = 79.01728024042073173;
   expected_results [ 2 ][ 2 ][ 2 ] = 79.091209616829445395;
   expected_results [ 2 ][ 3 ][ 0 ] = 99999;
   expected_results [ 2 ][ 3 ][ 1 ] = 99999;
   expected_results [ 2 ][ 3 ][ 2 ] = 99999;
   expected_results [ 2 ][ 4 ][ 0 ] = 77.31690458302028901;
   expected_results [ 2 ][ 4 ][ 1 ] = 77.390833959428988464;
   expected_results [ 2 ][ 4 ][ 2 ] = 77.46476333583770213;
   expected_results [ 2 ][ 5 ][ 0 ] = 99999;
   expected_results [ 2 ][ 5 ][ 1 ] = 99999;
   expected_results [ 2 ][ 5 ][ 2 ] = 99999;
   expected_results [ 2 ][ 6 ][ 0 ] = 75.690458302028559956;
   expected_results [ 2 ][ 6 ][ 1 ] = 75.76438767843725941;
   expected_results [ 2 ][ 6 ][ 2 ] = 75.838317054845973075;
   expected_results [ 2 ][ 7 ][ 0 ] = 99999;
   expected_results [ 2 ][ 7 ][ 1 ] = 99999;
   expected_results [ 2 ][ 7 ][ 2 ] = 99999;
   expected_results [ 2 ][ 8 ][ 0 ] = 74.064012021036816691;
   expected_results [ 2 ][ 8 ][ 1 ] = 74.137941397445530356;
   expected_results [ 2 ][ 8 ][ 2 ] = 74.211870773854244021;
   expected_results [ 2 ][ 9 ][ 0 ] = 99999;
   expected_results [ 2 ][ 9 ][ 1 ] = 99999;
   expected_results [ 2 ][ 9 ][ 2 ] = 99999;
   expected_results [ 2 ][ 10 ][ 0 ] = 72.437565740045073426;
   expected_results [ 2 ][ 10 ][ 1 ] = 72.511495116453787091;
   expected_results [ 2 ][ 10 ][ 2 ] = 72.585424492862514967;
   expected_results [ 3 ][ 0 ][ 0 ] = 99999;
   expected_results [ 3 ][ 0 ][ 1 ] = 99999;
   expected_results [ 3 ][ 0 ][ 2 ] = 99999;
   expected_results [ 3 ][ 1 ][ 0 ] = 70.811119459053330161;
   expected_results [ 3 ][ 1 ][ 1 ] = 70.885048835462043826;
   expected_results [ 3 ][ 1 ][ 2 ] = 70.958978211870771702;
   expected_results [ 3 ][ 2 ][ 0 ] = 99999;
   expected_results [ 3 ][ 2 ][ 1 ] = 99999;
   expected_results [ 3 ][ 2 ][ 2 ] = 99999;
   expected_results [ 3 ][ 3 ][ 0 ] = 69.184673178061615317;
   expected_results [ 3 ][ 3 ][ 1 ] = 69.258602554470314772;
   expected_results [ 3 ][ 3 ][ 2 ] = 69.332531930879028437;
   expected_results [ 3 ][ 4 ][ 0 ] = 99999;
   expected_results [ 3 ][ 4 ][ 1 ] = 99999;
   expected_results [ 3 ][ 4 ][ 2 ] = 99999;
   expected_results [ 3 ][ 5 ][ 0 ] = 67.558226897069857841;
   expected_results [ 3 ][ 5 ][ 1 ] = 67.632156273478585717;
   expected_results [ 3 ][ 5 ][ 2 ] = 67.706085649887299382;
   expected_results [ 3 ][ 6 ][ 0 ] = 99999;
   expected_results [ 3 ][ 6 ][ 1 ] = 99999;
   expected_results [ 3 ][ 6 ][ 2 ] = 99999;
   expected_results [ 3 ][ 7 ][ 0 ] = 65.931780616078128787;
   expected_results [ 3 ][ 7 ][ 1 ] = 66.005709992486842452;
   expected_results [ 3 ][ 7 ][ 2 ] = 66.079639368895556117;
   expected_results [ 3 ][ 8 ][ 0 ] = 99999;
   expected_results [ 3 ][ 8 ][ 1 ] = 99999;
   expected_results [ 3 ][ 8 ][ 2 ] = 99999;
   expected_results [ 3 ][ 9 ][ 0 ] = 64.305334335086399733;
   expected_results [ 3 ][ 9 ][ 1 ] = 64.379263711495099187;
   expected_results [ 3 ][ 9 ][ 2 ] = 64.453193087903812852;
   expected_results [ 3 ][ 10 ][ 0 ] = 99999;
   expected_results [ 3 ][ 10 ][ 1 ] = 99999;
   expected_results [ 3 ][ 10 ][ 2 ] = 99999;
   expected_results [ 4 ][ 0 ][ 0 ] = 62.678888054094670679;
   expected_results [ 4 ][ 0 ][ 1 ] = 62.752817430503377238;
   expected_results [ 4 ][ 0 ][ 2 ] = 62.826746806912083798;
   expected_results [ 4 ][ 1 ][ 0 ] = 99999;
   expected_results [ 4 ][ 1 ][ 1 ] = 99999;
   expected_results [ 4 ][ 1 ][ 2 ] = 99999;
   expected_results [ 4 ][ 2 ][ 0 ] = 61.052441773102927414;
   expected_results [ 4 ][ 2 ][ 1 ] = 61.126371149511648184;
   expected_results [ 4 ][ 2 ][ 2 ] = 61.200300525920354744;
   expected_results [ 4 ][ 3 ][ 0 ] = 99999;
   expected_results [ 4 ][ 3 ][ 1 ] = 99999;
   expected_results [ 4 ][ 3 ][ 2 ] = 99999;
   expected_results [ 4 ][ 4 ][ 0 ] = 59.425995492111184149;
   expected_results [ 4 ][ 4 ][ 1 ] = 59.499924868519904919;
   expected_results [ 4 ][ 4 ][ 2 ] = 59.57385424492862569;
   expected_results [ 4 ][ 5 ][ 0 ] = 99999;
   expected_results [ 4 ][ 5 ][ 1 ] = 99999;
   expected_results [ 4 ][ 5 ][ 2 ] = 99999;
   expected_results [ 4 ][ 6 ][ 0 ] = 57.799549211119455094;
   expected_results [ 4 ][ 6 ][ 1 ] = 57.873478587528168759;
   expected_results [ 4 ][ 6 ][ 2 ] = 57.94740796393688953;
   expected_results [ 4 ][ 7 ][ 0 ] = 99999;
   expected_results [ 4 ][ 7 ][ 1 ] = 99999;
   expected_results [ 4 ][ 7 ][ 2 ] = 99999;
   expected_results [ 4 ][ 8 ][ 0 ] = 56.17310293012772604;
   expected_results [ 4 ][ 8 ][ 1 ] = 56.2470323065364326;
   expected_results [ 4 ][ 8 ][ 2 ] = 56.320961682945146265;
   expected_results [ 4 ][ 9 ][ 0 ] = 99999;
   expected_results [ 4 ][ 9 ][ 1 ] = 99999;
   expected_results [ 4 ][ 9 ][ 2 ] = 99999;
   expected_results [ 4 ][ 10 ][ 0 ] = 54.546656649135989881;
   expected_results [ 4 ][ 10 ][ 1 ] = 54.620586025544710651;
   expected_results [ 4 ][ 10 ][ 2 ] = 54.694515401953410105;
   expected_results [ 5 ][ 0 ][ 0 ] = 99999;
   expected_results [ 5 ][ 0 ][ 1 ] = 99999;
   expected_results [ 5 ][ 0 ][ 2 ] = 99999;
   expected_results [ 5 ][ 1 ][ 0 ] = 52.92021036814423951;
   expected_results [ 5 ][ 1 ][ 1 ] = 52.994139744552960281;
   expected_results [ 5 ][ 1 ][ 2 ] = 53.068069120961681051;
   expected_results [ 5 ][ 2 ][ 0 ] = 99999;
   expected_results [ 5 ][ 2 ][ 1 ] = 99999;
   expected_results [ 5 ][ 2 ][ 2 ] = 99999;
   expected_results [ 5 ][ 3 ][ 0 ] = 51.293764087152510456;
   expected_results [ 5 ][ 3 ][ 1 ] = 51.367693463561217015;
   expected_results [ 5 ][ 3 ][ 2 ] = 51.441622839969937786;
   expected_results [ 5 ][ 4 ][ 0 ] = 99999;
   expected_results [ 5 ][ 4 ][ 1 ] = 99999;
   expected_results [ 5 ][ 4 ][ 2 ] = 99999;
   expected_results [ 5 ][ 5 ][ 0 ] = 49.667317806160774296;
   expected_results [ 5 ][ 5 ][ 1 ] = 49.741247182569495067;
   expected_results [ 5 ][ 5 ][ 2 ] = 49.815176558978215837;
   expected_results [ 5 ][ 6 ][ 0 ] = 99999;
   expected_results [ 5 ][ 6 ][ 1 ] = 99999;
   expected_results [ 5 ][ 6 ][ 2 ] = 99999;
   expected_results [ 5 ][ 7 ][ 0 ] = 48.040871525169038136;
   expected_results [ 5 ][ 7 ][ 1 ] = 48.114800901577758907;
   expected_results [ 5 ][ 7 ][ 2 ] = 48.188730277986472572;
   expected_results [ 5 ][ 8 ][ 0 ] = 99999;
   expected_results [ 5 ][ 8 ][ 1 ] = 99999;
   expected_results [ 5 ][ 8 ][ 2 ] = 99999;
   expected_results [ 5 ][ 9 ][ 0 ] = 46.414425244177301977;
   expected_results [ 5 ][ 9 ][ 1 ] = 46.488354620586022747;
   expected_results [ 5 ][ 9 ][ 2 ] = 46.562283996994736412;
   expected_results [ 5 ][ 10 ][ 0 ] = 99999;
   expected_results [ 5 ][ 10 ][ 1 ] = 99999;
   expected_results [ 5 ][ 10 ][ 2 ] = 99999;
   expected_results [ 6 ][ 0 ][ 0 ] = 44.787978963185572923;
   expected_results [ 6 ][ 0 ][ 1 ] = 44.861908339594293693;
   expected_results [ 6 ][ 0 ][ 2 ] = 44.935837716003000253;
   expected_results [ 6 ][ 1 ][ 0 ] = 99999;
   expected_results [ 6 ][ 1 ][ 1 ] = 99999;
   expected_results [ 6 ][ 1 ][ 2 ] = 99999;
   expected_results [ 6 ][ 2 ][ 0 ] = 43.161532682193829658;
   expected_results [ 6 ][ 2 ][ 1 ] = 43.235462058602550428;
   expected_results [ 6 ][ 2 ][ 2 ] = 43.309391435011271199;
   expected_results [ 6 ][ 3 ][ 0 ] = 99999;
   expected_results [ 6 ][ 3 ][ 1 ] = 99999;
   expected_results [ 6 ][ 3 ][ 2 ] = 99999;
   expected_results [ 6 ][ 4 ][ 0 ] = 41.535086401202100603;
   expected_results [ 6 ][ 4 ][ 1 ] = 41.609015777610821374;
   expected_results [ 6 ][ 4 ][ 2 ] = 41.682945154019527934;
   expected_results [ 6 ][ 5 ][ 0 ] = 99999;
   expected_results [ 6 ][ 5 ][ 1 ] = 99999;
   expected_results [ 6 ][ 5 ][ 2 ] = 99999;
   expected_results [ 6 ][ 6 ][ 0 ] = 39.908640120210357338;
   expected_results [ 6 ][ 6 ][ 1 ] = 39.982569496619078109;
   expected_results [ 6 ][ 6 ][ 2 ] = 40.056498873027798879;
   expected_results [ 6 ][ 7 ][ 0 ] = 99999;
   expected_results [ 6 ][ 7 ][ 1 ] = 99999;
   expected_results [ 6 ][ 7 ][ 2 ] = 99999;
   expected_results [ 6 ][ 8 ][ 0 ] = 38.282193839218628284;
   expected_results [ 6 ][ 8 ][ 1 ] = 38.356123215627349055;
   expected_results [ 6 ][ 8 ][ 2 ] = 38.430052592036055614;
   expected_results [ 6 ][ 9 ][ 0 ] = 99999;
   expected_results [ 6 ][ 9 ][ 1 ] = 99999;
   expected_results [ 6 ][ 9 ][ 2 ] = 99999;
   expected_results [ 6 ][ 10 ][ 0 ] = 36.655747558226885019;
   expected_results [ 6 ][ 10 ][ 1 ] = 36.729676934635605789;
   expected_results [ 6 ][ 10 ][ 2 ] = 36.80360631104432656;
   expected_results [ 7 ][ 0 ][ 0 ] = 99999;
   expected_results [ 7 ][ 0 ][ 1 ] = 99999;
   expected_results [ 7 ][ 0 ][ 2 ] = 99999;
   expected_results [ 7 ][ 1 ][ 0 ] = 35.029301277235141754;
   expected_results [ 7 ][ 1 ][ 1 ] = 35.103230653643876735;
   expected_results [ 7 ][ 1 ][ 2 ] = 35.17716003005257619;
   expected_results [ 7 ][ 2 ][ 0 ] = 99999;
   expected_results [ 7 ][ 2 ][ 1 ] = 99999;
   expected_results [ 7 ][ 2 ][ 2 ] = 99999;
   expected_results [ 7 ][ 3 ][ 0 ] = 33.402854996243405594;
   expected_results [ 7 ][ 3 ][ 1 ] = 33.476784372652126365;
   expected_results [ 7 ][ 3 ][ 2 ] = 33.550713749060861346;
   expected_results [ 7 ][ 4 ][ 0 ] = 99999;
   expected_results [ 7 ][ 4 ][ 1 ] = 99999;
   expected_results [ 7 ][ 4 ][ 2 ] = 99999;
   expected_results [ 7 ][ 5 ][ 0 ] = 31.776408715251680093;
   expected_results [ 7 ][ 5 ][ 1 ] = 31.850338091660407969;
   expected_results [ 7 ][ 5 ][ 2 ] = 31.924267468069107423;
   expected_results [ 7 ][ 6 ][ 0 ] = 99999;
   expected_results [ 7 ][ 6 ][ 1 ] = 99999;
   expected_results [ 7 ][ 6 ][ 2 ] = 99999;
   expected_results [ 7 ][ 7 ][ 0 ] = 30.149962434259936828;
   expected_results [ 7 ][ 7 ][ 1 ] = 30.223891810668657598;
   expected_results [ 7 ][ 7 ][ 2 ] = 30.297821187077389027;
   expected_results [ 7 ][ 8 ][ 0 ] = 99999;
   expected_results [ 7 ][ 8 ][ 1 ] = 99999;
   expected_results [ 7 ][ 8 ][ 2 ] = 99999;
   expected_results [ 7 ][ 9 ][ 0 ] = 28.523516153268218432;
   expected_results [ 7 ][ 9 ][ 1 ] = 28.597445529676939202;
   expected_results [ 7 ][ 9 ][ 2 ] = 28.671374906085638656;
   expected_results [ 7 ][ 10 ][ 0 ] = 99999;
   expected_results [ 7 ][ 10 ][ 1 ] = 99999;
   expected_results [ 7 ][ 10 ][ 2 ] = 99999;
   expected_results [ 8 ][ 0 ][ 0 ] = 26.897069872276489377;
   expected_results [ 8 ][ 0 ][ 1 ] = 26.97099924868519949;
   expected_results [ 8 ][ 0 ][ 2 ] = 27.04492862509392026;
   expected_results [ 8 ][ 1 ][ 0 ] = 99999;
   expected_results [ 8 ][ 1 ][ 1 ] = 99999;
   expected_results [ 8 ][ 1 ][ 2 ] = 99999;
   expected_results [ 8 ][ 2 ][ 0 ] = 25.270623591284749665;
   expected_results [ 8 ][ 2 ][ 1 ] = 25.344552967693470436;
   expected_results [ 8 ][ 2 ][ 2 ] = 25.418482344102166337;
   expected_results [ 8 ][ 3 ][ 0 ] = 99999;
   expected_results [ 8 ][ 3 ][ 1 ] = 99999;
   expected_results [ 8 ][ 3 ][ 2 ] = 99999;
   expected_results [ 8 ][ 4 ][ 0 ] = 23.644177310293020611;
   expected_results [ 8 ][ 4 ][ 1 ] = 23.71810668670172717;
   expected_results [ 8 ][ 4 ][ 2 ] = 23.792036063110447941;
   expected_results [ 8 ][ 5 ][ 0 ] = 99999;
   expected_results [ 8 ][ 5 ][ 1 ] = 99999;
   expected_results [ 8 ][ 5 ][ 2 ] = 99999;
   expected_results [ 8 ][ 6 ][ 0 ] = 22.017731029301277346;
   expected_results [ 8 ][ 6 ][ 1 ] = 22.091660405709998116;
   expected_results [ 8 ][ 6 ][ 2 ] = 22.165589782118704676;
   expected_results [ 8 ][ 7 ][ 0 ] = 99999;
   expected_results [ 8 ][ 7 ][ 1 ] = 99999;
   expected_results [ 8 ][ 7 ][ 2 ] = 99999;
   expected_results [ 8 ][ 8 ][ 0 ] = 20.391284748309548291;
   expected_results [ 8 ][ 8 ][ 1 ] = 20.465214124718254851;
   expected_results [ 8 ][ 8 ][ 2 ] = 20.539143501126975622;
   expected_results [ 8 ][ 9 ][ 0 ] = 99999;
   expected_results [ 8 ][ 9 ][ 1 ] = 99999;
   expected_results [ 8 ][ 9 ][ 2 ] = 99999;
   expected_results [ 8 ][ 10 ][ 0 ] = 18.764838467317805026;
   expected_results [ 8 ][ 10 ][ 1 ] = 18.838767843726525797;
   expected_results [ 8 ][ 10 ][ 2 ] = 18.912697220135235909;
   expected_results [ 9 ][ 0 ][ 0 ] = 99999;
   expected_results [ 9 ][ 0 ][ 1 ] = 99999;
   expected_results [ 9 ][ 0 ][ 2 ] = 99999;
   expected_results [ 9 ][ 1 ][ 0 ] = 17.138392186326075972;
   expected_results [ 9 ][ 1 ][ 1 ] = 17.212321562734786085;
   expected_results [ 9 ][ 1 ][ 2 ] = 17.286250939143503302;
   expected_results [ 9 ][ 2 ][ 0 ] = 99999;
   expected_results [ 9 ][ 2 ][ 1 ] = 99999;
   expected_results [ 9 ][ 2 ][ 2 ] = 99999;
   expected_results [ 9 ][ 3 ][ 0 ] = 15.511945905334334483;
   expected_results [ 9 ][ 3 ][ 1 ] = 15.585875281743055254;
   expected_results [ 9 ][ 3 ][ 2 ] = 15.65980465815176359;
   expected_results [ 9 ][ 4 ][ 0 ] = 99999;
   expected_results [ 9 ][ 4 ][ 1 ] = 99999;
   expected_results [ 9 ][ 4 ][ 2 ] = 99999;
   expected_results [ 9 ][ 5 ][ 0 ] = 13.885499624342605429;
   expected_results [ 9 ][ 5 ][ 1 ] = 13.959429000751313765;
   expected_results [ 9 ][ 5 ][ 2 ] = 14.033358377160034536;
   expected_results [ 9 ][ 6 ][ 0 ] = 99999;
   expected_results [ 9 ][ 6 ][ 1 ] = 99999;
   expected_results [ 9 ][ 6 ][ 2 ] = 99999;
   expected_results [ 9 ][ 7 ][ 0 ] = 12.25905334335086394;
   expected_results [ 9 ][ 7 ][ 1 ] = 12.332982719759584711;
   expected_results [ 9 ][ 7 ][ 2 ] = 12.406912096168291271;
   expected_results [ 9 ][ 8 ][ 0 ] = 99999;
   expected_results [ 9 ][ 8 ][ 1 ] = 99999;
   expected_results [ 9 ][ 8 ][ 2 ] = 99999;
   expected_results [ 9 ][ 9 ][ 0 ] = 10.632607062359134886;
   expected_results [ 9 ][ 9 ][ 1 ] = 10.706536438767843222;
   expected_results [ 9 ][ 9 ][ 2 ] = 10.780465815176562216;
   expected_results [ 9 ][ 10 ][ 0 ] = 99999;
   expected_results [ 9 ][ 10 ][ 1 ] = 99999;
   expected_results [ 9 ][ 10 ][ 2 ] = 99999;
   expected_results [ 10 ][ 0 ][ 0 ] = 9.0061607813673933975;
   expected_results [ 10 ][ 0 ][ 1 ] = 9.0800901577761123917;
   expected_results [ 10 ][ 0 ][ 2 ] = 9.1540195341848225041;
   expected_results [ 10 ][ 1 ][ 0 ] = 99999;
   expected_results [ 10 ][ 1 ][ 1 ] = 99999;
   expected_results [ 10 ][ 1 ][ 2 ] = 99999;
   expected_results [ 10 ][ 2 ][ 0 ] = 7.3797145003756616788;
   expected_results [ 10 ][ 2 ][ 1 ] = 7.453643876784370903;
   expected_results [ 10 ][ 2 ][ 2 ] = 7.5275732531930916736;
   expected_results [ 10 ][ 3 ][ 0 ] = 99999;
   expected_results [ 10 ][ 3 ][ 1 ] = 99999;
   expected_results [ 10 ][ 3 ][ 2 ] = 99999;
   expected_results [ 10 ][ 4 ][ 0 ] = 5.7532682193839210782;
   expected_results [ 10 ][ 4 ][ 1 ] = 5.8271975957926409606;
   expected_results [ 10 ][ 4 ][ 2 ] = 5.9011269722013501848;
   expected_results [ 10 ][ 5 ][ 0 ] = 99999;
   expected_results [ 10 ][ 5 ][ 1 ] = 99999;
   expected_results [ 10 ][ 5 ][ 2 ] = 99999;
   expected_results [ 10 ][ 6 ][ 0 ] = 4.1268219383921911358;
   expected_results [ 10 ][ 6 ][ 1 ] = 4.2007513148009003601;
   expected_results [ 10 ][ 6 ][ 2 ] = 4.2746806912096202424;
   expected_results [ 10 ][ 7 ][ 0 ] = 99999;
   expected_results [ 10 ][ 7 ][ 1 ] = 99999;
   expected_results [ 10 ][ 7 ][ 2 ] = 99999;
   expected_results [ 10 ][ 8 ][ 0 ] = 2.5003756574004500912;
   expected_results [ 10 ][ 8 ][ 1 ] = 2.5743050338091699736;
   expected_results [ 10 ][ 8 ][ 2 ] = 2.6482344102178791978;
   expected_results [ 10 ][ 9 ][ 0 ] = 99999;
   expected_results [ 10 ][ 9 ][ 1 ] = 99999;
   expected_results [ 10 ][ 9 ][ 2 ] = 99999;
   expected_results [ 10 ][ 10 ][ 0 ] = 0.87392937640872003779;
   expected_results [ 10 ][ 10 ][ 1 ] = 0.94785875281742903997;
   expected_results [ 10 ][ 10 ][ 2 ] = 1.0217881292261490334;

#ifdef VALIDATION
   //These number sonly needed for the calculation of the validation numbers
   const double gammaRayScaleFactor=0.0158;
   const double gammaRayOffset=0.8;

   //Generate validation output
   std::cerr<<"These are the validation data:"<<std::endl;
   std::cerr<<"std::vector< std::vector< std::vector< double>> > expected_results("<<gammaRay->lastI ( true )+1 - gammaRay->firstI ( true )<<", std::vector< std::vector< double> > ( "<<gammaRay->lastJ ( true )+1-gammaRay->firstJ ( true )<<", std::vector<double>( " <<gammaRay->lengthK()<<" ) ) );"<<std::endl;
#endif

   for ( unsigned int i = gammaRay->firstI ( true ); i <= gammaRay->lastI ( true ); ++i ) {
      for ( unsigned int j = gammaRay->firstJ ( true ); j <= gammaRay->lastJ ( true ); ++j ) {

#ifdef VALIDATION       
         const double solidRadiogenicHeatProduction = 1.55472;
#endif
         
         for ( unsigned int k = gammaRay->firstK (); k <= gammaRay->lastK (); ++k ) {

#ifdef VALIDATION            
            //Generate validation output 
            if( (i+j) % 2 == 0){
               std::cerr<<"expected_results [ "<<i<<" ][ "<<j<<" ][ "<<k<<" ] = "<<std::setprecision(20)<<((((1 - (0.01 * porosity->getA ( i, j, k ))) * solidRadiogenicHeatProduction) / gammaRayScaleFactor) + gammaRayOffset)<<";"<<std::endl;;
            }else{
               std::cerr<<"expected_results [ "<<i<<" ][ "<<j<<" ][ "<<k<<" ] = "<<std::setprecision(20)<<undefinedValue<<";"<<std::endl;;
            }
#else
            EXPECT_DOUBLE_EQ ( gammaRay->get ( i, j, k ), expected_results[i][j][k] );
#endif

         }

      }

   }

}

