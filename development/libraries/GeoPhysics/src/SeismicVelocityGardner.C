#include "GeoPhysicalConstants.h"
#include "SeismicVelocityGardner.h"

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
		const double porosity) const
	{
		double velocity = pow(densityBulk / GardnerVelocityConstant, 4);
		return velocity;
	}

}