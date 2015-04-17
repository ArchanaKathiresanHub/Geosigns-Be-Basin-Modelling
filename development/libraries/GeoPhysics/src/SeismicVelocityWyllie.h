#ifndef _SEISMICVELOCITYWYLLIE_H_
#define _SEISMICVELOCITYWYLLIE_H_

#include "SeismicVelocity.h"

namespace GeoPhysics
{
	class SeismicVelocityWyllie : public SeismicVelocity::Algorithm
	{
	public:
		SeismicVelocityWyllie(double seismicVelocitySolid);
		virtual ~SeismicVelocityWyllie();

		virtual double seismicVelocity(const FluidType* fluid,
			const double density,
			const double porosity,
			const double porePressure,
			const double temperature) const;

		virtual SeismicVelocity::Model model() const;

	private:
		double m_seismicVelocitySolid;
	};
}

#endif