#ifndef _SEISMICVELOCITYWYLLIE_H_
#define _SEISMICVELOCITYWYLLIE_H_

#include "SeismicVelocity.h"

namespace GeoPhysics
{
	/*! \class SeismicVelocityWyllie
	* \brief  Derived class of SeismicVelocity::Algorithm using the Wyllies's model for the seismic velocity computation.
	*/
	class SeismicVelocityWyllie : public SeismicVelocity::Algorithm
	{
	public:
		SeismicVelocityWyllie(double seismicVelocitySolid);
		virtual ~SeismicVelocityWyllie();

		/*!
		* \brief Compute the seismicVelocity (of the bulk, inlc. prosity and fluid) using the Wyllies's model.
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

	private:
		/// The seismic velocity value of the lithology (of the solid part, excluding porosity and fluid).
		double m_seismicVelocitySolid;
	};
}

#endif