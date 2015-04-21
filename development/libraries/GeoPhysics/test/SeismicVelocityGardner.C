#include "../src/SeismicVelocityGardner.h"

#include <gtest/gtest.h>

using namespace GeoPhysics;

TEST( SeismicVelocityGardner, std_sandstone )
{
	// with std. sandstone
	SeismicVelocity myVelocity;
	myVelocity = myVelocity.create(DataAccess::Interface::GARDNERS_VELOCITY_ALGORITHM, 6097);

	//For different values of density
	EXPECT_NEAR(1498.4480742408, myVelocity.seismicVelocity(-1, 1925, 0.3), 1e-10);
	EXPECT_NEAR(2556.2862607622, myVelocity.seismicVelocity(-1, 2200, 0.3), 1e-10);
	EXPECT_NEAR(5381.4865390602, myVelocity.seismicVelocity(-1, 2650, 0.3), 1e-10);

	//For dummy zero density value
	EXPECT_EQ(0, myVelocity.seismicVelocity(-1, 0, 0.3));

	//For other unused parameters in Gardner mode (fluid seismic velociy and porosity)
	EXPECT_EQ(myVelocity.seismicVelocity(-1, 1925, 0.45), myVelocity.seismicVelocity(1507, 1925, 0.3));
}

TEST(SeismicVelocityGardner, std_shale )
{
	// with std. shale
	SeismicVelocity myVelocity;
	myVelocity = myVelocity.create(DataAccess::Interface::GARDNERS_VELOCITY_ALGORITHM, 5000);

	//For different values of density
	EXPECT_NEAR(629.8622137281, myVelocity.seismicVelocity(-1, 1550, 0.5), 1e-10);
	EXPECT_NEAR(2556.2862607622, myVelocity.seismicVelocity(-1, 2200, 0.5), 1e-10);
	EXPECT_NEAR(5885.6702410592, myVelocity.seismicVelocity(-1, 2710, 0.5), 1e-10);

	//For dummy zero density value
	EXPECT_EQ(0, myVelocity.seismicVelocity(-1, 0, 0.5));

	//For other unused parameters in Gardner mode (fluid seismic velociy and porosity)
	EXPECT_EQ(myVelocity.seismicVelocity(-1, 1550, 0.5), myVelocity.seismicVelocity(1507, 1550, 0.55));
}