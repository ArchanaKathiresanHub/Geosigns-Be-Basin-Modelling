//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SeismicVelocityWyllie.h"

namespace GeoPhysics
{
	SeismicVelocityWyllie::SeismicVelocityWyllie(const double seismicVelocitySolid) :
		Algorithm(),
		m_seismicVelocitySolid(seismicVelocitySolid)
	{}

	SeismicVelocityWyllie::~SeismicVelocityWyllie()
	{}

	double SeismicVelocityWyllie::calculate(const double seismicVelocityFluid,
		const double densityFluid,
		const double densityBulk,
		const double porosity,
		const double currentVes,
		const double maxVes) const
	{
		double velocity;
		//If there is a fluid
		if (seismicVelocityFluid != -1){
			double denominator = porosity * m_seismicVelocitySolid + (1.0 - porosity) * seismicVelocityFluid;
			assert( ("Dividing by zero using transit time average", denominator!=0) );
			velocity = (seismicVelocityFluid * m_seismicVelocitySolid) / denominator;
		}
		//Else, if there is no fluid
		else {
			velocity = m_seismicVelocitySolid;
		}
		return velocity;
	}
}