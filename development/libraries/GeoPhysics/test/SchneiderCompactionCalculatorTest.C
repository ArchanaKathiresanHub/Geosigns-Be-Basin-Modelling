/**
 * \file SchmeiderCompactionCalculatorTest.C
 * \brief Test the SchneiderCompactionCalculator class
 * \author Magali Cougnenc magali.cougnenc@pds.nl
 * \date June 2014
 *
 * Test of the chemical compaction through Schneider model
 * Test of extreme time values, extreme parameters values, no valid nodes...
 */


#include "../src/SchneiderCompactionCalculator.h"
#include "../src/GeoPhysicalConstants.h"
#include "MockGrid.h"
#include <gtest/gtest.h>
#include <cmath>
#include <iostream>


using namespace GeoPhysics;

class testSchneiderCompactionCalculator : public ::testing::Test
{
public:

	testSchneiderCompactionCalculator()  {;}
	~testSchneiderCompactionCalculator() {;}

	double computeTestTimeStep(
			double porosity,
			double ves,
			double time0,
			double time1,
			double activationEnergy,
			double referenceViscosity,
			double temperature,
			double chemicalCompaction )
	{
		const double temperatureTerm  = 1.0 / ( std::max( temperature , RockViscosityReferenceTemperature  ) + CelciusToKelvin ) - 1.0 / ( RockViscosityReferenceTemperature + CelciusToKelvin );
		const double solidViscosity   = 1.0e9 * referenceViscosity * exp( activationEnergy * temperatureTerm / GasConstant );
		double result           = (time1-time0) * ( 1.0 - porosity ) * ves / solidViscosity;
		result += chemicalCompaction;
		return result;
	}
};



/*!
 * \brief Test the input of short and long time steps
 */
TEST_F( testSchneiderCompactionCalculator, test_extreme_time_values )
{
	SchneiderCompactionCalculator * my_Object = dynamic_cast < SchneiderCompactionCalculator* > ( ChemicalCompactionCalculator::create ( "Schneider" ) );

	const int size           = 1;
	const int sizeValidNodes = 1;

	const double currentTime    = 20.0;     //[s]
	const double previousTime1  = 20.0;     //[s]
	const double previousTime2  = 20.5;     //[s]
	const double previousTime3  = 100020.0; //[s]

	double chemicalCompaction1 [size] = { 0.0 }; //[fraction of unit volume]
	double chemicalCompaction2 [size] = { 0.0 }; //[fraction of unit volume]
	double chemicalCompaction3 [size] = { 0.0 }; //[fraction of unit volume]
	const double porosity      [size] = { 0.3 }; //[fraction of unit volume]

	const double temperature [size] = { 100 }; //[C]
	const double ves         [size] = { 10.0e6}; //[Pa]

	const int validNodes [sizeValidNodes] = { 0 };
	const int lithoId    [size]           = { 0 };
	const int numberLithologies           = 1;

	const double activationEnergy              [numberLithologies] = { 15.0e3 }; //[J/mol]
	const double referenceViscosity            [numberLithologies] = { 100.0 };  //[GPa.my]


	MockGrid grid1( size, previousTime1, currentTime, chemicalCompaction1, porosity, temperature, ves, sizeValidNodes, validNodes,
			lithoId, numberLithologies, activationEnergy, referenceViscosity );

	MockGrid grid2( size, previousTime2, currentTime, chemicalCompaction2, porosity, temperature, ves, sizeValidNodes, validNodes,
			lithoId, numberLithologies, activationEnergy, referenceViscosity );

	MockGrid grid3( size, previousTime3, currentTime, chemicalCompaction3, porosity, temperature, ves, sizeValidNodes, validNodes,
			lithoId, numberLithologies, activationEnergy, referenceViscosity );

	my_Object -> computeOnTimeStep ( grid1 ); //Time step = 0 => no change in the chemical compaction
	ASSERT_EQ( chemicalCompaction1[0], grid1.setChemicalCompaction()[0] );

	my_Object -> computeOnTimeStep ( grid2 ); //Time step = 1 My => normal change in the chemical compaction
	double result = computeTestTimeStep( porosity[0], ves[0], previousTime2, currentTime,
			activationEnergy[0], referenceViscosity[0],
			temperature[0], chemicalCompaction1[0]);
	ASSERT_DOUBLE_EQ( result, grid2.setChemicalCompaction()[0] );

	my_Object -> computeOnTimeStep ( grid3 ); //Time step = 1000 My => chemical compaction > -1
	ASSERT_LE( grid3.setChemicalCompaction()[0], -1.0 );
}

/*!
 * \brief Test the input of minimal, mean and maximal values of porosity, temperature and vertical effective stress
 */
TEST_F( testSchneiderCompactionCalculator, test_extreme_parameters_values )
{
	//Cover range of values for temperature, ves and porosity
	SchneiderCompactionCalculator * my_Object = dynamic_cast < SchneiderCompactionCalculator* > ( ChemicalCompactionCalculator::create ( "Schneider" ) );

	const int size           = 45;
	const int sizeValidNodes = size;

	const double previousTime = 21.0; //[s]
	const double currentTime  = 20.0; //[s]

	double chemicalCompaction1 [size]; //[fraction of unit volume]
	double chemicalCompaction2 [size]; //[fraction of unit volume]
	double porosity            [size]; //[fraction of unit volume]

	std::fill_n( chemicalCompaction1, size, 0.0 );
	std::fill_n( chemicalCompaction2, size, 0.0 );
	std::fill_n( porosity,       15,      0.0  );
	std::fill_n( porosity + 15,  30,      0.3  );
	std::fill_n( porosity + 30,  size-30, 1.0  );

	double temperature [size]; //[celcius]
	double ves         [size]; //[Pa]

	for ( int index = 0; index < size; index += 3 )
	{
		switch( index % 15 )
		{
		case 0:
			temperature [index    ] = -273.15;
			temperature [index + 1] = -273.15;
			temperature [index + 2] = -273.15;
			ves         [index    ] = 0.0;
			ves         [index+1  ] = 50.0e6;
			ves         [index + 2] = 500.0e6;
			break;

		case 3:
			temperature [index    ] = 15.0;
			temperature [index + 1] = 15.0;
			temperature [index + 2] = 15.0;
			ves         [index    ] = 0.0;
			ves         [index+1  ] = 50.0e6;
			ves         [index + 2] = 500.0e6;
			break;

		case 6:
			temperature [index    ] = 100.0;
			temperature [index + 1] = 100.0;
			temperature [index + 2] = 100.0;
			ves         [index    ] = 0.0;
			ves         [index+1  ] = 50.0e6;
			ves         [index + 2] = 500.0e6;
			break;

		case 9:
			temperature [index    ] = 500.0;
			temperature [index + 1] = 500.0;
			temperature [index + 2] = 500.0;
			ves         [index    ] = 0.0;
			ves         [index+1  ] = 50.0e6;
			ves         [index + 2] = 500.0e6;
			break;

		case 12:
			temperature [index    ] = 10000.0;
			temperature [index + 1] = 10000.0;
			temperature [index + 2] = 10000.0;
			ves         [index    ] = 0.0;
			ves         [index+1  ] = 50.0e6;
			ves         [index + 2] = 500.0e6;
			break;
		}
	}


	int validNodes    [sizeValidNodes];
	for ( int i = 0 ; i < sizeValidNodes ; ++i ) { validNodes[i] = i; }
	const int lithoId [size]    = { 0 };
	const int numberLithologies = 1;

	const double activationEnergy              [numberLithologies] = { 15.0e3 };             //[J/mol]
	const double referenceViscosity            [numberLithologies] = { 100.0 };              //[GPa.my]

	MockGrid grid( size, previousTime, currentTime, chemicalCompaction1, porosity, temperature, ves, sizeValidNodes, validNodes,
			lithoId, numberLithologies, activationEnergy, referenceViscosity );

	my_Object -> computeOnTimeStep( grid );

	for ( int i=0 ; i<size; ++i )
	{
		double result         = grid.setChemicalCompaction()[i];
		double expectedResult = 0;
		if( porosity[i] > MinimumPorosity )
		{
			expectedResult = std::max(
					computeTestTimeStep( porosity[i], ves[i], previousTime, currentTime,
							activationEnergy[0], referenceViscosity[0],
							temperature[i], chemicalCompaction2[i]) ,
							-1.0 );
		}
		else
		{
			expectedResult = chemicalCompaction1[i];
		}
		ASSERT_GE( 0.0, result );                       // chemical compaction <= 0
		ASSERT_LE( -1.0, result );                      // chemical compaction >= -1
		ASSERT_GE( chemicalCompaction1[i], result );    // chemical compaction (t + deltat) <= chemical compaction (t)
		ASSERT_DOUBLE_EQ( expectedResult, result );     // chemical compaction = expected chemical compaction
	}
}

/*!
 * \brief Test that nothing happens if not a valid node
 */
TEST_F( testSchneiderCompactionCalculator, no_valid_nodes )
{
	// No valid nodes => no chemical compaction
	SchneiderCompactionCalculator * my_Object = dynamic_cast < SchneiderCompactionCalculator* > ( ChemicalCompactionCalculator::create ( "Schneider" ) );

	const int size           = 2;
	const int sizeValidNodes = 1;

	const double previousTime = 21.0; //[s]
	const double currentTime  = 20.0; //[s]

	double chemicalCompaction1 [size] = { 0.0, 0.0 }; //[fraction of unit volume]
	const double porosity      [size] = { 0.3, 0.3 }; //[fraction of unit volume]

	const double temperature [size] = { 100, 100 };       //[celsius]
	const double ves         [size] = { 50.0e6, 50.0e6 }; //[Pa]

	int validNodes    [sizeValidNodes] = { 0 };
	const int lithoId [size]           = { 0 };
	const int numberLithologies        = 1;

	const double activationEnergy              [numberLithologies] = { 15.0e3 };               //[J/mol]
	const double referenceViscosity            [numberLithologies] = { 100.0 };                //[GPa.my]

	MockGrid grid( size, previousTime, currentTime, chemicalCompaction1, porosity, temperature, ves, sizeValidNodes, validNodes,
			lithoId, numberLithologies, activationEnergy, referenceViscosity );

	my_Object -> computeOnTimeStep ( grid );

	ASSERT_GT( 0.0, grid.setChemicalCompaction()[0] ); // If valid node => compaction
	ASSERT_EQ( 0.0, grid.setChemicalCompaction()[1] ); //If non valid node => no compaction
}

/*!
 * \brief Test input temperature close to reference temperature
 */
TEST_F( testSchneiderCompactionCalculator, reference_temperature )
{
	SchneiderCompactionCalculator * my_Object = dynamic_cast < SchneiderCompactionCalculator* > ( ChemicalCompactionCalculator::create ( "Schneider" ) );

	const int numberLithologies = 1;

	const double activationEnergy              [numberLithologies] = { 15.0e3 };                //[J/mol]
	const double referenceViscosity            [numberLithologies] = { 1000.0 };                //[GPa.my]

	const int size           = 4;
	const int sizeValidNodes = 4;

	const double previousTime = 21.0; //[s]
	const double currentTime  = 20.0; //[s]

	double chemicalCompaction1 [size] = { 0.0, 0.0, 0.0, 0.0 }; //[fraction of unit volume]
	const double      porosity [size] = { 0.3, 0.3, 0.3, 0.3 }; //[fraction of unit volume]

	double temperature [size] = { 0, RockViscosityReferenceTemperature - 1e-10, RockViscosityReferenceTemperature, RockViscosityReferenceTemperature - 1e-10 }; //[K]
	for (int i =0 ; i < size ; ++i ){ temperature[i] -= CelciusToKelvin; }                               // [C]
	const double ves         [size] = { 50.0e6, 50.0e6, 50.0e6, 50.0e6 };                                //[Pa]

	int validNodes    [sizeValidNodes] = { 0, 1, 2, 3 };
	const int lithoId [size]           = { 0 };

	MockGrid grid( size, previousTime, currentTime, chemicalCompaction1, porosity, temperature, ves, sizeValidNodes, validNodes,
			lithoId, numberLithologies, activationEnergy, referenceViscosity );

	double result = ( chemicalCompaction1[0] ) - ( ( 1 - porosity[0] ) * ves[0] / ( 1.0e9* referenceViscosity[0] ) * ( previousTime - currentTime ));

	my_Object -> computeOnTimeStep ( grid );
	ASSERT_EQ( result, grid.setChemicalCompaction()[0] );        // T = 0 => exponential part = 1
	ASSERT_EQ( result, grid.setChemicalCompaction()[1] );        // T --> T0 with T<T0 => exponential part = 1
	ASSERT_EQ( result, grid.setChemicalCompaction()[2] );        // T = TO => exponential part = 1
	ASSERT_DOUBLE_EQ( result, grid.setChemicalCompaction()[3] ); // T --> T0 with T>T0 => exponential part --> 1
}

/*!
 * \brief Test that different lithologies lead to different results
 */
TEST_F( testSchneiderCompactionCalculator, extreme_lithology_values )
{
	SchneiderCompactionCalculator * my_Object = dynamic_cast < SchneiderCompactionCalculator* > ( ChemicalCompactionCalculator::create ( "Schneider" ) );

	const int numberLithologies = 9;

	const double activationEnergy [numberLithologies] = { 0.0,15.0e3,100.0e3,0.0,15.0e3,100.0e3,0.0,15.0e3,100.0e3 };       //[J/mol]
	double referenceViscosity     [numberLithologies] = { 0.3,100.0,300.0,0.3,100.0,300.0,0.3,100.0,300.0 };                //[GPa.my]

	const int size           = 9;
	const int sizeValidNodes = size;

	const double previousTime = 21.0; //[ma]
	const double currentTime  = 20.0; //[ma]

	double chemicalCompaction1 [size] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }; //[fraction of unit volume]
	//Same than chemical compaction, allow to compare the expected and computed results
	double chemicalCompaction2 [size] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }; //[fraction of unit volume]
	const double porosity      [size] = { 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3 }; //[fraction of unit volume]

	const double temperature [size] = { 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0, 100.0 };          //[C]
	const double ves         [size] = { 50.0e6, 50.0e6, 50.0e6, 50.0e6, 50.0e6, 50.0e6, 50.0e6, 50.0e6, 50.0e6 }; //[Pa]

	int validNodes    [sizeValidNodes] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
	const int lithoId [size]           = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };

	MockGrid grid( size, previousTime, currentTime, chemicalCompaction1, porosity, temperature, ves, sizeValidNodes, validNodes,
			lithoId, numberLithologies, activationEnergy, referenceViscosity );

	my_Object -> computeOnTimeStep ( grid );

	for ( int i=0 ; i<size; ++i )
	{
		double expectedResult = std::max(
				computeTestTimeStep( porosity[i], ves[i], previousTime, currentTime,
						activationEnergy[i], referenceViscosity[i],
						temperature[i], chemicalCompaction2[i]) ,
						-1.0 );
		double result         = grid.setChemicalCompaction()[i];
		ASSERT_GE(  0.0, result );                      // chemical compaction <= 0
		ASSERT_LE( -1.0, result );                      // chemical compaction >= -1
		ASSERT_GE( chemicalCompaction1[i], result );    // chemical compaction (t + deltat) <= chemical compaction (t)
		ASSERT_DOUBLE_EQ( expectedResult, result );     //chemical compaction
	}
}

TEST_F( testSchneiderCompactionCalculator, test_change_lithology )
{
	SchneiderCompactionCalculator * my_Object = dynamic_cast < SchneiderCompactionCalculator* > ( ChemicalCompactionCalculator::create ( "Schneider" ) );

	const int size           = 1;
	const int sizeValidNodes = size;

	const double previousTime = 22.0 ; //[ma]
	const double currentTime1 = 21.0 ; //[ma]
	const double currentTime2 = 20.0 ; //[ma]

	double chemicalCompaction1 [size] = {0.0}; //[fraction of unit volume]
	double chemicalCompaction2 [size] = {0.0}; //[fraction of unit volume]
	double chemicalCompaction3 [size] = {0.0}; //[fraction of unit volume]
	const double porosity1     [size] = {0.5}; //[fraction of unit volume]
	const double porosity2     [size] = {0.3}; //[fraction of unit volume]

	const double temperature1 [size] = {80.0};  //[C]
	const double temperature2 [size] = {81.0};  //[C]
	const double ves          [size] = {50.0e6}; //[Pa]

	const int validNodes[sizeValidNodes] = {0};
	int lithoId         [size]           = {0};
	const int numberLithologies          = 1;

	const double activationEnergy1    [numberLithologies] = { 15.0e3 };                //[J/mol]
	const double referenceViscosity1  [numberLithologies] = { 1000.0 };                //[GPa.my]

	const double activationEnergy2    [numberLithologies] = { 15.0e3 };                //[J/mol]
	const double referenceViscosity2  [numberLithologies] = { 10000.0 };               //[GPa.my]


	MockGrid previousGrid( size, previousTime, currentTime1, chemicalCompaction1, porosity1, temperature1, ves, sizeValidNodes, validNodes,
			lithoId, numberLithologies, activationEnergy1, referenceViscosity1 );

	MockGrid currentGrid1( size, currentTime1, currentTime2, chemicalCompaction2, porosity1, temperature1, ves, sizeValidNodes, validNodes,
			lithoId, numberLithologies, activationEnergy2, referenceViscosity2 );

	// The change of lithology is taken into account
	double expectedResult1 = computeTestTimeStep( porosity1[0], ves[0], previousTime, currentTime1,
			activationEnergy1[0], referenceViscosity1[0],
			temperature1[0], chemicalCompaction1[0] );
	my_Object -> computeOnTimeStep ( previousGrid );
	double result1         = previousGrid.setChemicalCompaction()[0];
	ASSERT_DOUBLE_EQ( expectedResult1, result1 );
	ASSERT_DOUBLE_EQ( result1, chemicalCompaction1[0] );


	double expectedResult2 = computeTestTimeStep( porosity1[0], ves[0], currentTime1, currentTime2,
			activationEnergy2[0], referenceViscosity2[0],
			temperature1[0], chemicalCompaction2[0] );
	my_Object -> computeOnTimeStep ( currentGrid1 );
	double result2         = currentGrid1.setChemicalCompaction()[0];

	ASSERT_DOUBLE_EQ( result2, chemicalCompaction2[0] );
	ASSERT_DOUBLE_EQ( result2, expectedResult2 );



}
