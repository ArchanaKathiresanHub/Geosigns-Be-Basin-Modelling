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
		* \param fluid The seismic velocity of the fluid (must be -1 if there is no fluid).
		* \param density The bulk density.
		* \param porosity The porosity.
		*/
		virtual double seismicVelocity(const double seismciVelocityFluid,
			const double density,
			const double porosity) const;
	};
}

#endif