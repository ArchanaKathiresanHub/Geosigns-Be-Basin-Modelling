#ifndef _SEISMICVELOCITYGARDNER_H_
#define _SEISMICVELOCITYGARDNER_H_

#include "SeismicVelocity.h"


namespace GeoPhysics
{
	/*! \class SeismicVelocityGardner
	* \brief  Derived class of SeismicVelocity::Algorithm using the Garnder's model for the seismic velocity computation.
	*/
	class SeismicVelocityGardner : public SeismicVelocity::Algorithm
	{
	public:
		SeismicVelocityGardner();
		virtual ~SeismicVelocityGardner();

		/*!
		* \brief Compute the seismicVelocity (of the bulk, inlc. prosity and fluid) using the Garnder's model.
		* \param fluid The fluid (must be a NULL pointer if there is no fluid).
		* \param density The bulk density.
		* \param porePressure The pore pressure.
		* \param temperature The temperature.
		*/
		virtual double seismicVelocity(const FluidType* fluid,
			const double density,
			const double porosity,
			const double porePressure,
			const double temperature) const;
	};
}

#endif