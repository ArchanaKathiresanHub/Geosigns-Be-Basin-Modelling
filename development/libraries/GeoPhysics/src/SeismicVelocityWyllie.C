#include "GeoPhysicalConstants.h"
#include "SeismicVelocityWyllie.h"

namespace GeoPhysics
{
	SeismicVelocityWyllie::SeismicVelocityWyllie(const double seismicVelocitySolid) :
		Algorithm(),
		m_seismicVelocitySolid(seismicVelocitySolid)
	{}

	SeismicVelocityWyllie::~SeismicVelocityWyllie()
	{}

	double SeismicVelocityWyllie::seismicVelocity(const double seismicVelocityFluid,
		const double densityFluid,
		const double densityBulk,
		const double porosity) const
	{
		double velocity;
		//If there is a fluid
		if (seismicVelocityFluid != -1){
			double denominator = porosity * m_seismicVelocitySolid + (1.0 - porosity) * seismicVelocityFluid;
			assert(denominator!=0);
			velocity = (seismicVelocityFluid * m_seismicVelocitySolid) / denominator;
		}
		//Else, if there is no fluid
		else {
			velocity = m_seismicVelocitySolid;
		}
		return velocity;
	}
}