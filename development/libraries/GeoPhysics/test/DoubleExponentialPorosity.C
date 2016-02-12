#include "../src/DoubleExponentialPorosity.h"
#include "../src/ExponentialPorosity.h"

#include <gtest/gtest.h>
#include <algorithm>
#include <cmath>
#include <iostream>


using namespace GeoPhysics;

namespace {
const double NaN = std::numeric_limits<double>::quiet_NaN();
}

TEST( DoubleExponentialPorosity, sub_arkose )
{
	// with a typical sub-arkose
	Porosity myPorosity( Porosity::create(DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY, 0.41, 0.0, 0.0, 1.631e-8, 3.874e-8, 0.0, 0.1631e-8, 0.3874e-8, 0.0 ));

	//For different values of ves
	EXPECT_NEAR( 0.41 , myPorosity.calculate( 0.0e6, 0.0e6,false, 0.0 ), 1e-10);
	EXPECT_NEAR( 0.35784629093113574 , myPorosity.calculate( 5.0e6, 5.0e6,false, 0.0 ), 1e-10);
	EXPECT_NEAR( 0.12024352325 , myPorosity.calculate( 5.0e7, 5.0e7,false, 0.0 ), 1e-10);
	EXPECT_NEAR( 0.00005889633 , myPorosity.calculate( 5.0e8, 5.0e8,false, 0.0 ), 1e-10);
	
	//For different values of chemical compaction
	EXPECT_NEAR( 0.12024352325 , myPorosity.calculate( 5.0e7, 5.0e7,true, 0.0 ), 1e-10);
	EXPECT_NEAR( 0.03 , myPorosity.calculate( 5.0e7, 5.0e7, true, -1.0 ), 1e-10);

	//If there is a minimumMechanicalPorosity
	Porosity myPorosity2( Porosity::create(DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY, 0.41, 0.05, 0.0, 1.631e-8, 3.874e-8, 0.0, 0.1631e-8, 0.3874e-8, 0.0 ));

	EXPECT_NEAR( 0.05 , myPorosity2.calculate( 5.0e10, 5.0e10,false, 0.0 ), 1e-10);
	//If there is both mechanical and chemical => chemical only is taken into account
	EXPECT_NEAR( 0.03 , myPorosity2.calculate( 5.0e7, 5.0e7,true, -1.0 ), 1e-10);
	
}

TEST( DoubleExponentialPorosity, extreme_coefficients_and_porosity )
{
	// with a typical chalk
	Porosity typicalChalk( Porosity::create(DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY, 0.70, 0.0, 0.0, 5.000e-6, 0.045e-6, 0.0, 5.000e-7, 0.045e-7, 0.0 ));
	EXPECT_NEAR( 0.03688972860 , typicalChalk.calculate( 50.0e6, 50.0e6,false, 0.0 ), 1e-10);
	
	// with an ooid grainstone limestone
	Porosity ooidGraistone( Porosity::create(DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY, 0.35, 0.0, 0.0, 0.001e-6, 0.001e-6, 0.0, 0.001e-7, 0.001e-7, 0.0 ));
	EXPECT_NEAR( 0.34825436772, ooidGraistone.calculate( 5.0e6, 5.0e6,false, 0.0 ), 1e-10);
	
	//with nul deposition porosity
	Porosity nulPorosity( Porosity::create(DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY, 0.0, 0.0, 0.0, 5.000e-6, 0.045e-6, 0.0, 5.000e-7, 0.045e-7, 0.0 ));
	EXPECT_NEAR( 0.0 , nulPorosity.calculate( 50.0e6, 50.0e6,false, 0.0 ), 1e-10);
		
	//with nul coefficients => no compaction
	Porosity nulCoefficients( Porosity::create(DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY, 0.70, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 ));
	EXPECT_NEAR( 0.70 , nulCoefficients.calculate( 50.0e6, 50.0e6,false, 0.0 ), 1e-10);
	
	//with extreme coefficients => porosity = minimum porosity
	Porosity extremeCoefficients( Porosity::create(DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY, 0.70, 0.05, 0.0, 1.0, 1.0, 0.0, 0.1, 0.1, 0.0 ));
	EXPECT_NEAR( 0.05 , extremeCoefficients.calculate( 50.0e6, 50.0e6,false, 0.0 ), 1e-10);
		
		
}


TEST( DoubleExponentialPorosity, elastic_rebound )
{
	// with a typical sub-arkose
	Porosity elasticRebound( Porosity::create(DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY, 0.41, 0.0, 0.0, 1.631e-8, 3.874e-8, 0.0, 0.1631e-8, 0.3874e-8, 0.0 ));
	double withoutRebound = elasticRebound.calculate( 40.0e6, 40.0e6, false, 0.0 );
	double withoutRebound2 = elasticRebound.calculate( 50.0e6, 50.0e6, false, 0.0 );
	double withRebound = elasticRebound.calculate( 40.0e6, 50.0e6, false, 0.0 );
	
	//test the relations
	EXPECT_GT(withoutRebound, withRebound);
	EXPECT_GT(withRebound, withoutRebound2);
	
	//and test the values
	EXPECT_NEAR( 0.15029108813291073, withoutRebound, 1e-10);
	EXPECT_NEAR( 0.12024352325186743, withoutRebound2, 1e-10);
	EXPECT_NEAR( 0.12290204345369726, withRebound, 1e-10);
}

TEST( DoubleExponentialPorosity, single_is_special_case_double )
{
	//typical sandstone
	Porosity singleExp( Porosity::create(DataAccess::Interface::EXPONENTIAL_POROSITY, 0.41, 0.0, 0.0266e-6, 0.0, 0.0, 0.0266e-7, 0.0, 0.0, 0.0 ));
	Porosity doubleExp( Porosity::create(DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY, 0.41, 0.0, 0.0, 0.0266e-6, 0.0266e-6, 0.0, 0.0266e-7, 0.0266e-7, 0.0 ));
	
	EXPECT_NEAR( doubleExp.calculate( 50.0e6, 50.0e6,false, 0.0 ), singleExp.calculate( 50.0e6, 50.0e6,false, 0.0 ), 1e-10);
	EXPECT_NEAR( doubleExp.calculate( 40.0e6, 50.0e6,false, 0.0 ), singleExp.calculate( 40.0e6, 50.0e6,false, 0.0 ), 1e-10);
}

