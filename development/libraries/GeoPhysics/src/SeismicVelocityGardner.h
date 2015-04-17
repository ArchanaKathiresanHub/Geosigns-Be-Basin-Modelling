#ifndef _SEISMICVELOCITYGARDNER_H_
#define _SEISMICVELOCITYGARDNER_H_

#include "SeismicVelocity.h"


namespace GeoPhysics
{
	class SeismicVelocityGardner : public SeismicVelocity::Algorithm
	{
	public:
		SeismicVelocityGardner();
		virtual ~SeismicVelocityGardner();

		virtual double seismicVelocity(const FluidType* fluid,
			const double density,
			const double porosity,
			const double porePressure,
			const double temperature) const;

		virtual SeismicVelocity::Model model() const;
	};
}

#endif