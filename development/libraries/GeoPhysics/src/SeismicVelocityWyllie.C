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

	double SeismicVelocityWyllie::seismicVelocity(const FluidType* fluid,
		const double density,
		const double porosity,
		const double porePressure,
		const double temperature) const
	{
		double velocity;
			if (fluid != 0) {
				double seismciVelocitySolid;
				seismciVelocitySolid = fluid->seismicVelocity(temperature, porePressure);
				assert(porosity * m_seismicVelocitySolid + (1.0 - porosity) * seismciVelocitySolid != 0);
				velocity = (seismciVelocitySolid * m_seismicVelocitySolid) / (porosity * m_seismicVelocitySolid + (1.0 - porosity) * seismciVelocitySolid);
			}
			else {
				velocity = m_seismicVelocitySolid;
			}
			return velocity;
	}
}