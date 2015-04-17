#include "GeoPhysicalConstants.h"
#include "SeismicVelocityGardner.h"

namespace GeoPhysics
{
	SeismicVelocityGardner::SeismicVelocityGardner() :
		Algorithm()
	{}

	SeismicVelocityGardner::~SeismicVelocityGardner()
	{}

	double SeismicVelocityGardner::seismicVelocity(const FluidType* fluid,
		const double density,
		const double porosity,
		const double porePressure,
		const double temperature) const
	{
		double velocity = pow(density / GardnerVelocityConstant, 4);
		return velocity;
	}

}