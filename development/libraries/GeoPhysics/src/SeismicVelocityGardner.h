#ifndef _SEISMICVELOCITYGARDNER_H_
#define _SEISMICVELOCITYGARDNER_H_

#include "SeismicVelocity.h"


namespace GeoPhysics
{
	/*! \class SeismicVelocityGardner
	* \brief  Derived class of SeismicVelocity::Algorithm using the Garnder's model for the seismic velocity computation.
	* \details \f[ Vp = (\frac{ \rho_{bulk} }{ G })^4 \f] where \f[G = 309.4\f]
	*	G is the Gardner constant.
	*/
	class SeismicVelocityGardner : public SeismicVelocity::Algorithm
	{
	public:
		SeismicVelocityGardner();
		virtual ~SeismicVelocityGardner();

		/*!
		* \brief Compute the seismic velocity of the bulk (inlc. prosity and fluid) using the Garnder's model.
		* \param seismicVelocityFluid The seismic velocity of the fluid (-1 if there is no fluid).
		* \param densityFluid The fluid density (-1 if there is no fluid).
		* \param densityBulk The bulk density (inlc. prosity and fluid).
		* \param porosity The porosity.
		* \param currentVes The current vertical effective stress.
		* \param maxVes The maximum vertical effective stress.
		* \warning Only densityBulk parameter is used for the Gardner computation mode.
		*/
		virtual double seismicVelocity(const double seismicVelocityFluid,
			const double densityFluid,
			const double densityBulk,
			const double porosity,
			const double currentVes,
			const double maxVes) const;
	};
}

#endif