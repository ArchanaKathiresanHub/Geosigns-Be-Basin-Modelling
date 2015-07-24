#include "../src/SeismicVelocityLorcan.h"

#include <gtest/gtest.h>

using namespace GeoPhysics;

TEST(SeismicVelocityLorcan, std_sandstone_compaction)
{
	// with std. sandstone
	SeismicVelocity myVelocity;
	myVelocity = myVelocity.create(DataAccess::Interface::KENNANS_VELOCITY_ALGORITHM, 6097, 98509533850, 2650, 0.48, 0.75);

	//Compaction when ves=vesMax

	//For different values of fluid seismic velocity
	EXPECT_NEAR(3689.3000183922, myVelocity.seismicVelocity(1000, 1000, 2200, 0.3, 40, 40), 1e-10);
	EXPECT_NEAR(3856.4949216494, myVelocity.seismicVelocity(1500, 1000, 2200, 0.3, 40, 40), 1e-10);
	EXPECT_NEAR(4044.6211463954, myVelocity.seismicVelocity(2000, 1000, 2200, 0.3, 40, 40), 1e-10);

	//For different values of fluid density
	EXPECT_NEAR(3859.0065309790, myVelocity.seismicVelocity(1507, 1000, 2200, 0.3, 40, 40), 1e-10);
	EXPECT_NEAR(3831.4880445658, myVelocity.seismicVelocity(1507, 900, 2200, 0.3, 40, 40), 1e-10);
	EXPECT_NEAR(3802.9672639059, myVelocity.seismicVelocity(1507, 800, 2200, 0.3, 40, 40), 1e-10);

	//For no fluid
	EXPECT_EQ(6097, myVelocity.seismicVelocity(-1, -1, 2200, 0.3, 40, 40));
	EXPECT_EQ(6097, myVelocity.seismicVelocity(-1, -1, 2650, 0.45, 40, 40));

	//For different values of bulk density
	EXPECT_NEAR(4125.4515120704, myVelocity.seismicVelocity(1507, 1000, 1925, 0.3, 40, 40), 1e-10);
	EXPECT_NEAR(3859.0065309790, myVelocity.seismicVelocity(1507, 1000, 2200, 0.3, 40, 40), 1e-10);
	EXPECT_NEAR(3516.1218502028, myVelocity.seismicVelocity(1507, 1000, 2650, 0.3, 40, 40), 1e-10);

	//For different values of porosity
	EXPECT_NEAR(3859.0065309790, myVelocity.seismicVelocity(1507, 1000, 2200, 0.3, 40, 40), 1e-10);
	EXPECT_NEAR(2659.7690226332, myVelocity.seismicVelocity(1507, 1000, 2200, 0.4, 40, 40), 1e-10);
	EXPECT_NEAR(1946.7685572242, myVelocity.seismicVelocity(1507, 1000, 2200, 0.45, 40, 40), 1e-10);

	//For suspension line (porosity > surfacePorosity)
	EXPECT_EQ(1507, myVelocity.seismicVelocity(1507, 1000, 2200, 0.5, 40, 40));
}

TEST(SeismicVelocityLorcan, std_sandstone_decompaction)
{
	// with std. sandstone
	SeismicVelocity myVelocity;
	myVelocity = myVelocity.create(DataAccess::Interface::KENNANS_VELOCITY_ALGORITHM, 6097, 98509533850, 2650, 0.48, 0.75);

	//Decompaction when ves<vesMax

	//For different values of fluid seismic velocity
	EXPECT_NEAR(2455.4345654028, myVelocity.seismicVelocity(1000, 1000, 2200, 0.3, 0, 40), 1e-10);
	EXPECT_NEAR(2756.8245312314, myVelocity.seismicVelocity(1500, 1000, 2200, 0.3, 0, 40), 1e-10);
	EXPECT_NEAR(3097.5255216299, myVelocity.seismicVelocity(2000, 1000, 2200, 0.3, 0, 40), 1e-10);

	//For different values of fluid density
	EXPECT_NEAR(2761.3957624214, myVelocity.seismicVelocity(1507, 1000, 2200, 0.3, 0, 40), 1e-10);
	EXPECT_NEAR(2711.3254950292, myVelocity.seismicVelocity(1507, 900, 2200, 0.3, 0, 40), 1e-10);
	EXPECT_NEAR(2659.5048935019, myVelocity.seismicVelocity(1507, 800, 2200, 0.3, 0, 40), 1e-10);

	//For no fluid
	EXPECT_EQ(6097, myVelocity.seismicVelocity(-1, -1, 2200, 0.3, 0, 40));
	EXPECT_EQ(6097, myVelocity.seismicVelocity(-1, -1, 2650, 0.45, 0, 40));

	//For different values of bulk density
	EXPECT_NEAR(2952.0562435058, myVelocity.seismicVelocity(1507, 1000, 1925, 0.3, 0, 40), 1e-10);
	EXPECT_NEAR(2761.3957624214, myVelocity.seismicVelocity(1507, 1000, 2200, 0.3, 0, 40), 1e-10);
	EXPECT_NEAR(2516.0371974919, myVelocity.seismicVelocity(1507, 1000, 2650, 0.3, 0, 40), 1e-10);

	//For different values of porosity
	EXPECT_NEAR(2761.3957624214, myVelocity.seismicVelocity(1507, 1000, 2200, 0.3, 0, 40), 1e-10);
	EXPECT_NEAR(1804.9353406292, myVelocity.seismicVelocity(1507, 1000, 2200, 0.4, 0, 40), 1e-10);
	EXPECT_NEAR(1525.9574516777, myVelocity.seismicVelocity(1507, 1000, 2200, 0.45, 0, 40), 1e-10);

	//For different values of the current vertical effective stress (at fixed porosity)
	EXPECT_NEAR(3072.7776960063, myVelocity.seismicVelocity(1507, 1000, 2200, 0.3, 10, 40), 1e-10);
	EXPECT_NEAR(3355.3865621458, myVelocity.seismicVelocity(1507, 1000, 2200, 0.3, 20, 40), 1e-10);
	EXPECT_NEAR(3615.9749990540, myVelocity.seismicVelocity(1507, 1000, 2200, 0.3, 30, 40), 1e-10);

	//For different values of the maximum vertical effective stress (at fixed porosity)
	EXPECT_NEAR(2761.3957624214, myVelocity.seismicVelocity(1507, 1000, 2200, 0.3, 0, 50), 1e-10);
	EXPECT_NEAR(2761.3957624214, myVelocity.seismicVelocity(1507, 1000, 2200, 0.3, 0, 60), 1e-10);
	EXPECT_NEAR(2761.3957624214, myVelocity.seismicVelocity(1507, 1000, 2200, 0.3, 0, 70), 1e-10);

	//For suspension line (porosity > surfacePorosity)
	EXPECT_EQ(1507, myVelocity.seismicVelocity(1507, 1000, 2200, 0.5, 0, 40));
}

TEST(SeismicVelocityLorcan, std_shale_compaction)
{
	// with std. shale
	SeismicVelocity myVelocity;
	myVelocity = myVelocity.create(DataAccess::Interface::KENNANS_VELOCITY_ALGORITHM, 5000, 67750000000, 2710, 0.7, -0.75);

	//Compaction when ves=vesMax

	//For different values of fluid seismic velocity
	EXPECT_NEAR(1039.1457804505, myVelocity.seismicVelocity(1000, 1000, 2000, 0.5, 40, 40), 1e-10);
	EXPECT_NEAR(1534.4632773539, myVelocity.seismicVelocity(1500, 1000, 2000, 0.5, 40, 40), 1e-10);
	EXPECT_NEAR(2007.6802546729, myVelocity.seismicVelocity(2000, 1000, 2000, 0.5, 40, 40), 1e-10);

	//For different values of fluid density
	EXPECT_NEAR(1541.2483626245, myVelocity.seismicVelocity(1507, 1000, 2000, 0.5, 40, 40), 1e-10);
	EXPECT_NEAR(1466.0483021710, myVelocity.seismicVelocity(1507, 900, 2000, 0.5, 40, 40), 1e-10);
	EXPECT_NEAR(1385.9753871862, myVelocity.seismicVelocity(1507, 800, 2000, 0.5, 40, 40), 1e-10);

	//For no fluid
	EXPECT_EQ(5000, myVelocity.seismicVelocity(-1, -1, 2000, 0.5, 40, 40));
	EXPECT_EQ(5000, myVelocity.seismicVelocity(-1, -1, 2710, 0.6, 40, 40));

	//For different values of density
	EXPECT_NEAR(1750.7404941329, myVelocity.seismicVelocity(1507, 1000, 1550, 0.5, 40, 40), 1e-10);
	EXPECT_NEAR(1469.5226544984, myVelocity.seismicVelocity(1507, 1000, 2200, 0.5, 40, 40), 1e-10);
	EXPECT_NEAR(1324.0457164526, myVelocity.seismicVelocity(1507, 1000, 2710, 0.5, 40, 40), 1e-10);

	//For different values of porosity
	EXPECT_NEAR(1636.9171004820, myVelocity.seismicVelocity(1507, 1000, 2000, 0.45, 40, 40), 1e-10);
	EXPECT_NEAR(1458.4335489023, myVelocity.seismicVelocity(1507, 1000, 2000, 0.55, 40, 40), 1e-10);
	EXPECT_NEAR(1321.7868476107, myVelocity.seismicVelocity(1507, 1000, 2000, 0.65, 40, 40), 1e-10);
	
	//For suspension line (porosity > surfacePorosity)
	EXPECT_EQ(1507, myVelocity.seismicVelocity(1507, 1000, 2000, 0.8, 40, 40));
}

TEST(SeismicVelocityLorcan, std_shale_decompaction)
{
	// with std. shale
	SeismicVelocity myVelocity;
	myVelocity = myVelocity.create(DataAccess::Interface::KENNANS_VELOCITY_ALGORITHM, 5000, 67750000000, 2710, 0.7, -0.75);

	//Decompaction when ves<vesMax

	//For different values of fluid seismic velocity
	EXPECT_NEAR(1006.1894619467, myVelocity.seismicVelocity(1000, 1000, 2000, 0.5, 0, 40), 1e-10);
	EXPECT_NEAR(1492.7227259627, myVelocity.seismicVelocity(1500, 1000, 2000, 0.5, 0, 40), 1e-10);
	EXPECT_NEAR(1962.0170900526, myVelocity.seismicVelocity(2000, 1000, 2000, 0.5, 0, 40), 1e-10);

	//For different values of fluid density
	EXPECT_NEAR(1499.4210423884, myVelocity.seismicVelocity(1507, 1000, 2000, 0.5, 0, 40), 1e-10);
	EXPECT_NEAR(1425.2331673068, myVelocity.seismicVelocity(1507, 900, 2000, 0.5, 0, 40), 1e-10);
	EXPECT_NEAR(1346.3604142306, myVelocity.seismicVelocity(1507, 800, 2000, 0.5, 0, 40), 1e-10);

	//For no fluid
	EXPECT_EQ(5000, myVelocity.seismicVelocity(-1, -1, 2000, 0.5, 0, 40));
	EXPECT_EQ(5000, myVelocity.seismicVelocity(-1, -1, 2710, 0.6, 0, 40));

	//For different values of density
	EXPECT_NEAR(1703.2278510872, myVelocity.seismicVelocity(1507, 1000, 1550, 0.5, 0, 40), 1e-10);
	EXPECT_NEAR(1429.6418694450, myVelocity.seismicVelocity(1507, 1000, 2200, 0.5, 0, 40), 1e-10);
	EXPECT_NEAR(1288.1129715867, myVelocity.seismicVelocity(1507, 1000, 2710, 0.5, 0, 40), 1e-10);

	//For different values of porosity
	EXPECT_NEAR(1585.9685312837, myVelocity.seismicVelocity(1507, 1000, 2000, 0.45, 0, 40), 1e-10);
	EXPECT_NEAR(1426.4175320649, myVelocity.seismicVelocity(1507, 1000, 2000, 0.55, 0, 40), 1e-10);
	EXPECT_NEAR(1310.8056114186, myVelocity.seismicVelocity(1507, 1000, 2000, 0.65, 0, 40), 1e-10);

	//For different values of the current vertical effective stress (at fixed porosity)
	EXPECT_NEAR(1509.9864984797, myVelocity.seismicVelocity(1507, 1000, 2000, 0.5, 10, 40), 1e-10);
	EXPECT_NEAR(1520.4785394161, myVelocity.seismicVelocity(1507, 1000, 2000, 0.5, 20, 40), 1e-10);
	EXPECT_NEAR(1530.8986746544, myVelocity.seismicVelocity(1507, 1000, 2000, 0.5, 30, 40), 1e-10);

	//For different values of the maximum vertical effective stress (at fixed porosity)
	EXPECT_NEAR(1499.4210423884, myVelocity.seismicVelocity(1507, 1000, 2000, 0.5, 0, 50), 1e-10);
	EXPECT_NEAR(1499.4210423884, myVelocity.seismicVelocity(1507, 1000, 2000, 0.5, 0, 60), 1e-10);
	EXPECT_NEAR(1499.4210423884, myVelocity.seismicVelocity(1507, 1000, 2000, 0.5, 0, 70), 1e-10);

	//For suspension line (porosity > surfacePorosity)
	EXPECT_EQ(1507, myVelocity.seismicVelocity(1507, 1000, 2000, 0.8, 0, 40));
}

TEST(SeismicVelocityLorcan, death_test)
{
	::testing::FLAGS_gtest_death_test_style = "threadsafe";

	// divide by zero crash test
	SeismicVelocity myVelocity;
	myVelocity = myVelocity.create(DataAccess::Interface::KENNANS_VELOCITY_ALGORITHM, 5000, 0, 2710, 0.7, -0.75);
	ASSERT_DEATH(myVelocity.seismicVelocity(0, 1000, 2000, 0, 40, 40), "Assertion.*denominator!=0");
}