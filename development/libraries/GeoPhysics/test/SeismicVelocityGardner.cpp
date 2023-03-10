#include "../src/SeismicVelocityGardner.h"

#include <gtest/gtest.h>

using namespace GeoPhysics;

TEST( SeismicVelocityGardner, std_sandstone )
{
	// with std. sandstone
   SeismicVelocity myVelocity( SeismicVelocity::create( DataAccess::Interface::GARDNERS_VELOCITY_ALGORITHM, 6097, 2650, 98509533850, 0.48, 0.75 ) );

	//For different values of bulk density
	EXPECT_NEAR(1498.4480742408, myVelocity.calculate(-1, -1, 1925, 0.3, 40, 40), 1e-10);
	EXPECT_NEAR(2556.2862607622, myVelocity.calculate(-1, -1, 2200, 0.3, 40, 40), 1e-10);
	EXPECT_NEAR(5381.4865390602, myVelocity.calculate(-1, -1, 2650, 0.3, 40, 40), 1e-10);

	//For dummy zero density value
   EXPECT_EQ( 0, myVelocity.calculate( -1, -1, 0, 0.3, 40, 40 ) );

	//For other unused parameters in Gardner mode (fluid seismic velociy, fluid density and porosity)
   EXPECT_EQ( myVelocity.calculate( -1, -1, 1925, 0.45, 0, 40 ), myVelocity.calculate( 1507, 1000, 1925, 0.3, 50, 50 ) );
}

TEST(SeismicVelocityGardner, std_shale )
{
	// with std. shale
	SeismicVelocity myVelocity;
	myVelocity = myVelocity.create(DataAccess::Interface::GARDNERS_VELOCITY_ALGORITHM, 5000, 2710, 67750000000, 0.7, -0.75);

	//For different values of density
   EXPECT_NEAR( 629.8622137281, myVelocity.calculate( -1, -1, 1550, 0.5, 40, 40 ), 1e-10 );
   EXPECT_NEAR( 2556.2862607622, myVelocity.calculate( -1, -1, 2200, 0.5, 40, 40 ), 1e-10 );
   EXPECT_NEAR( 5885.6702410592, myVelocity.calculate( -1, -1, 2710, 0.5, 40, 40 ), 1e-10 );

	//For dummy zero density value
   EXPECT_EQ( 0, myVelocity.calculate( -1, -1, 0, 0.5, 40, 40 ) );

	//For other unused parameters in Gardner mode (fluid seismic velociy, fluid density and porosity)
   EXPECT_EQ( myVelocity.calculate( -1, -1, 1550, 0.5, 0, 40 ), myVelocity.calculate( 1507, 1000, 1550, 0.55, 50, 50 ) );
}