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