//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <iostream>
#include <cmath>
#include <cassert>
#include <cstdlib>

#include "../src/GeoPhysicsProjectHandle.h"
#include "../src/GeoPhysicsObjectFactory.h"

#include <gtest/gtest.h>
using namespace GeoPhysics;

TEST(MobileLayerInvalidMobLayThikn, missingThkn)
{
	GeoPhysics::ObjectFactory factory;
	ObjectFactory* factoryptr = &factory;
	GeoPhysics::ProjectHandle* ph = nullptr;
	try
	{
		ph = dynamic_cast<GeoPhysics::ProjectHandle*>(DataAccess::Interface::OpenCauldronProject("Project_mobLay.project3d",
			factoryptr));
		bool started = ph->startActivity("test", ph->getHighResolutionOutputGrid());
		if (!started) FAIL();
		// this call will throw the expection
		ph->initialise();
	}
	catch (std::exception& ex) {
		EXPECT_STREQ("Invalid thickness entry encountered for this layer, L2, at 10 Ma. check MobLayThicknIoTbl!", ex.what());
		SUCCEED();
	}
	catch (...)
	{
		delete ph;
		FAIL();
	}
}