#include "../src/SeismicVelocityLorcan.h"

#include <gtest/gtest.h>

using namespace GeoPhysics;

TEST( SeismicVelocityWyllie, std_sandstone )
{
	// with std. sandstone
	SeismicVelocity myVelocity;
	myVelocity = myVelocity.create(DataAccess::Interface::LORCANS_VELOCITY_ALGORITHM, 6097, 2650, 0.48, 0.75);

	//For different values of fluid seismic velocity
	EXPECT_NEAR(3689.3000183922, myVelocity.seismicVelocity(1000, 1000, 2200, 0.3), 1e-10);
	EXPECT_NEAR(3856.4949216494, myVelocity.seismicVelocity(1500, 1000, 2200, 0.3), 1e-10);
	EXPECT_NEAR(4044.6211463954, myVelocity.seismicVelocity(2000, 1000, 2200, 0.3), 1e-10);

	//For different values of fluid density
	EXPECT_NEAR(3859.0065309790, myVelocity.seismicVelocity(1507, 1000, 2200, 0.3), 1e-10);
	EXPECT_NEAR(3831.4880445658, myVelocity.seismicVelocity(1507, 900, 2200, 0.3), 1e-10);
	EXPECT_NEAR(3802.9672639059, myVelocity.seismicVelocity(1507, 800, 2200, 0.3), 1e-10);

	//For no fluid
	EXPECT_EQ(6097, myVelocity.seismicVelocity(-1, -1, 2200, 0.3));
	EXPECT_EQ(6097, myVelocity.seismicVelocity(-1, -1, 2650, 0.45));

	//For different values of bulk density
	EXPECT_NEAR(4125.4515120704, myVelocity.seismicVelocity(1507, 1000, 1925, 0.3), 1e-10);
	EXPECT_NEAR(3859.0065309790, myVelocity.seismicVelocity(1507, 1000, 2200, 0.3), 1e-10);
	EXPECT_NEAR(3516.1218502028, myVelocity.seismicVelocity(1507, 1000, 2650, 0.3), 1e-10);

	//For different values of porosity
	EXPECT_NEAR(3859.0065309790, myVelocity.seismicVelocity(1507, 1000, 2200, 0.3), 1e-10);
	EXPECT_NEAR(2659.7690226332, myVelocity.seismicVelocity(1507, 1000, 2200, 0.4), 1e-10);
	EXPECT_NEAR(1946.7685572242, myVelocity.seismicVelocity(1507, 1000, 2200, 0.45), 1e-10);

	//For suspension line (porosity > surfacePorosity)
	EXPECT_EQ(1507, myVelocity.seismicVelocity(1507, 1000, 2200, 0.5), 1e-10);

}

TEST(SeismicVelocityWyllie, std_shale)
{
	// with std. shale
	SeismicVelocity myVelocity;
	myVelocity = myVelocity.create(DataAccess::Interface::LORCANS_VELOCITY_ALGORITHM, 5000, 2710, 0.7, -0.75);

	//For different values of fluid seismic velocity
	EXPECT_NEAR(1039.1457804505, myVelocity.seismicVelocity(1000, 1000, 2000, 0.5), 1e-10);
	EXPECT_NEAR(1534.4632773539, myVelocity.seismicVelocity(1500, 1000, 2000, 0.5), 1e-10);
	EXPECT_NEAR(2007.6802546729, myVelocity.seismicVelocity(2000, 1000, 2000, 0.5), 1e-10);

	//For different values of fluid density
	EXPECT_NEAR(1541.2483626245, myVelocity.seismicVelocity(1507, 1000, 2000, 0.5), 1e-10);
	EXPECT_NEAR(1466.0483021710, myVelocity.seismicVelocity(1507, 900, 2000, 0.5), 1e-10);
	EXPECT_NEAR(1385.9753871862, myVelocity.seismicVelocity(1507, 800, 2000, 0.5), 1e-10);

	//For no fluid
	EXPECT_EQ(5000, myVelocity.seismicVelocity(-1, -1, 2000, 0.5));
	EXPECT_EQ(5000, myVelocity.seismicVelocity(-1, -1, 2710, 0.6));

	//For different values of density
	EXPECT_NEAR(1750.7404941329, myVelocity.seismicVelocity(1507, 1000, 1550, 0.5), 1e-10);
	EXPECT_NEAR(1469.5226544984, myVelocity.seismicVelocity(1507, 1000, 2200, 0.5), 1e-10);
	EXPECT_NEAR(1324.0457164526, myVelocity.seismicVelocity(1507, 1000, 2710, 0.5), 1e-10);

	//For different values of porosity
	EXPECT_NEAR(1636.9171004820, myVelocity.seismicVelocity(1507, 1000, 2000, 0.45), 1e-10);
	EXPECT_NEAR(1458.4335489023, myVelocity.seismicVelocity(1507, 1000, 2000, 0.55), 1e-10);
	EXPECT_NEAR(1321.7868476107, myVelocity.seismicVelocity(1507, 1000, 2000, 0.65), 1e-10);
	
	//For suspension line (porosity > surfacePorosity)
	EXPECT_EQ(1507, myVelocity.seismicVelocity(1507, 1000, 2000, 0.8), 1e-10);
}

TEST(SeismicVelocityWyllie, death_test)
{
	::testing::FLAGS_gtest_death_test_style = "threadsafe";

	// divide by zero crash test
	SeismicVelocity myVelocity;
	myVelocity = myVelocity.create(DataAccess::Interface::LORCANS_VELOCITY_ALGORITHM, 0, 2710, 0.7, -0.75);
	ASSERT_DEATH(myVelocity.seismicVelocity(0, 1000, 2000, 0), "Assertion.*denominator!=0");
}