#include "../src/SeismicVelocityWyllie.h"

#include <gtest/gtest.h>

using namespace GeoPhysics;

TEST( SeismicVelocityWyllie, std_sandstone )
{
	// with std. sandstone
   SeismicVelocity myVelocity( SeismicVelocity::create( DataAccess::Interface::WYLLIES_VELOCITY_ALGORITHM, 6097, 2650, 98509533850, 0.48, 0.75 ) );

	//For different values of fluid seismic velocity
	EXPECT_NEAR(2410.7389980626, myVelocity.calculate(1000, 1000, 2200, 0.3, 40, 40), 1e-10);
	EXPECT_NEAR(3176.5134938002, myVelocity.calculate(1500, 1000, 2200, 0.3, 40, 40), 1e-10);
	EXPECT_NEAR(3776.2844136137, myVelocity.calculate(2000, 1000, 2200, 0.3, 40, 40), 1e-10);

	//For no fluid
	EXPECT_EQ(6097, myVelocity.calculate(-1, -1, 2200, 0.3, 40, 40));
	EXPECT_EQ(6097, myVelocity.calculate(-1, -1, 2650, 0.45, 40, 40));

	//For different values of porosity
	EXPECT_NEAR(3185.9150485437, myVelocity.calculate(1507, 1000, 2200, 0.3, 40, 40), 1e-10);
	EXPECT_NEAR(2748.4830990129, myVelocity.calculate(1507, 1000, 2200, 0.4, 40, 40), 1e-10);
	EXPECT_NEAR(2571.9185444367, myVelocity.calculate(1507, 1000, 2200, 0.45, 40, 40), 1e-10);

	//For dummy zero fluid seismic velocity value
	EXPECT_EQ(0, myVelocity.calculate(0, 1000, 2200, 0.3, 40, 40));

	//For other unused parameters in Wyllie mode (fluid density and bulk density)
	EXPECT_EQ(myVelocity.calculate(1507, 1000, 2200, 0.3, 0, 40), myVelocity.calculate(1507, 800, 2650, 0.3, 50, 50));
}

TEST(SeismicVelocityWyllie, std_shale)
{
	// with std. shale
   SeismicVelocity myVelocity( SeismicVelocity::create( DataAccess::Interface::WYLLIES_VELOCITY_ALGORITHM, 5000, 2710, 67750000000, 0.7, -0.75 ) );

	//For different values of fluid seismic velocity
	EXPECT_NEAR(1666.6666666667, myVelocity.calculate(1000, 1000, 2000, 0.5, 40, 40), 1e-10);
	EXPECT_NEAR(2307.6923076923, myVelocity.calculate(1500, 1000, 2000, 0.5, 40, 40), 1e-10);
	EXPECT_NEAR(2857.1428571429, myVelocity.calculate(2000, 1000, 2000, 0.5, 40, 40), 1e-10);

	//For no fluid
	EXPECT_EQ(5000, myVelocity.calculate(-1, -1, 2000, 0.5, 40, 40));
	EXPECT_EQ(5000, myVelocity.calculate(-1, -1, 2710, 0.6, 40, 40));

	//For different values of porosity
	EXPECT_NEAR(2447.3423518522, myVelocity.calculate(1507, 1000, 2000, 0.45, 40, 40), 1e-10);
	EXPECT_NEAR(2197.9785015242, myVelocity.calculate(1507, 1000, 2000, 0.55, 40, 40), 1e-10);
	EXPECT_NEAR(1994.7318958557, myVelocity.calculate(1507, 1000, 2000, 0.65, 40, 40), 1e-10);

	//For dummy zero fluid seismic velocity value
	EXPECT_EQ(0, myVelocity.calculate(0, 1000, 2000, 0.5, 40, 40));

	//For other unused parameters in Wyllie mode (fluid density and bulk density)
	EXPECT_EQ(myVelocity.calculate(1507, 1000, 2000, 0.5, 0, 40), myVelocity.calculate(1507, 800, 1550, 0.5, 50, 50));
}

//assert is only handled in Debug
#ifndef NDEBUG
TEST(SeismicVelocityWyllie, death_test)
{
	::testing::FLAGS_gtest_death_test_style = "threadsafe";

	// divide by zero crash test
   SeismicVelocity myVelocity( SeismicVelocity::create( DataAccess::Interface::WYLLIES_VELOCITY_ALGORITHM, 5000, 2710, 67750000000, 0.7, 0 ) );
	ASSERT_DEATH(myVelocity.calculate(0, 1000, 2000, 0, 40, 40), "Assertion.*denominator!=0");
}
#endif