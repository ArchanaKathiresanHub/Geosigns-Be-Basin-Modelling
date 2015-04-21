#include "GeoPhysicalConstants.h"
#include "SeismicVelocityGardner.h"

namespace GeoPhysics
{
	SeismicVelocityGardner::SeismicVelocityGardner() :
		Algorithm()
	{}

	SeismicVelocityGardner::~SeismicVelocityGardner()
	{}

	double SeismicVelocityGardner::seismicVelocity(const double seismciVelocityFluid,
		const double density,
		const double porosity) const
	{
		double velocity = pow(density / GardnerVelocityConstant, 4);
		return velocity;
	}

}