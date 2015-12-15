//                                                                      
// Copyright (C) 2015-2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SeismicVelocityGardner.h"
#include "GeoPhysicalConstants.h"

namespace GeoPhysics
{
	SeismicVelocityGardner::SeismicVelocityGardner() :
		Algorithm()
	{}

	SeismicVelocityGardner::~SeismicVelocityGardner()
	{}

	double SeismicVelocityGardner::seismicVelocity(const double seismicVelocityFluid,
		const double densityFluid,
		const double densityBulk,
		const double porosity,
		const double currentVes,
		const double maxVes) const
	{
		// optimisation of pow(densityBulk / GardnerVelocityConstant, 4)
		double velocity = densityBulk / GardnerVelocityConstant;
		velocity *= velocity;
		velocity *= velocity;
		return velocity;
	}

}