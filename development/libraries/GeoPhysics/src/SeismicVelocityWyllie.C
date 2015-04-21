#include "GeoPhysicalConstants.h"
#include "SeismicVelocityWyllie.h"

namespace GeoPhysics
{
	SeismicVelocityWyllie::SeismicVelocityWyllie(double seismicVelocitySolid) :
		Algorithm(),
		m_seismicVelocitySolid(seismicVelocitySolid)
	{}

	SeismicVelocityWyllie::~SeismicVelocityWyllie()
	{}

	double SeismicVelocityWyllie::seismicVelocity(const double seismciVelocityFluid,
		const double density,
		const double porosity) const
	{
		double velocity;
		//If there is a fluid
		if (seismciVelocityFluid != -1){
			double denominator = porosity * m_seismicVelocitySolid + (1.0 - porosity) * seismciVelocityFluid;
			assert(denominator!=0);
			velocity = (seismciVelocityFluid * m_seismicVelocitySolid) / denominator;
		}
		//Else, if there is no fluid
		else {
			velocity = m_seismicVelocitySolid;
		}
		return velocity;
	}
}