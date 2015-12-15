/**
 * \file ChemicalCompactionCalculatorTest.C
 * \brief Test the ChemicalCompactionCalculator class
 * \author Magali Cougnenc magali.cougnenc@pds.nl
 * \date June 2014
 *
 * Test of the chemical compaction abstract class
 */

#include "../src/ChemicalCompactionCalculator.h"
#include "../src/SchneiderCompactionCalculator.h"
#include "../src/WalderhaugCompactionCalculator.h"
#include <gtest/gtest.h>

using namespace GeoPhysics;

/*
 * \brief Tests what the method create() returns according to input string
 */
TEST( testChemicalCompactionCalculator, testing_create )
{
   //If valid name and matching object
   ChemicalCompactionCalculator * my_Object1 = dynamic_cast<SchneiderCompactionCalculator*>( ChemicalCompactionCalculator::create( "Schneider" ) );
   EXPECT_TRUE( my_Object1 != NULL );

   ChemicalCompactionCalculator * my_Object2 = dynamic_cast<WalderhaugCompactionCalculator*>( ChemicalCompactionCalculator::create( "Walderhaug" ) );
   EXPECT_TRUE( my_Object2 != NULL );

   //If valid name but not matching object
   ChemicalCompactionCalculator * my_Object3 = dynamic_cast<WalderhaugCompactionCalculator*>( ChemicalCompactionCalculator::create( "Schneider" ) );
   EXPECT_TRUE( my_Object3 == NULL );

   ChemicalCompactionCalculator * my_Object4 = dynamic_cast<SchneiderCompactionCalculator*>( ChemicalCompactionCalculator::create( "Walderhaug" ) );
   EXPECT_TRUE( my_Object4 == NULL );

   //If not valid name
   ChemicalCompactionCalculator * my_Object5 = dynamic_cast<WalderhaugCompactionCalculator*>( ChemicalCompactionCalculator::create( "blablabla" ) );
   EXPECT_TRUE( my_Object5 == NULL );

   ChemicalCompactionCalculator * my_Object6 = dynamic_cast<SchneiderCompactionCalculator*>( ChemicalCompactionCalculator::create( "blablabla" ) );
   EXPECT_TRUE( my_Object6 == NULL );

}
