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
		* \brief Compute the seismic velocity of the bulk (inlc. prosity and fluid) using the Garnder's model.
		* \param seismicVelocityFluid The seismic velocity of the fluid (-1 if there is no fluid).
		* \param densityFluid The fluid density (-1 if there is no fluid).
		* \param densityBulk The bulk density (inlc. prosity and fluid).
		* \param porosity The porosity.
		* \warning Paramaters seismicVelocityFluid, densityFluid and porosity are not used for the Gardner computation mode.
		*/
		virtual double seismicVelocity(const double seismicVelocityFluid,
			const double densityFluid,
			const double densityBulk,
			const double porosity) const;
	};
}

#endif