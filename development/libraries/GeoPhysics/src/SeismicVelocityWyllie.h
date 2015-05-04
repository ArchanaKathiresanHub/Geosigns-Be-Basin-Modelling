#ifndef _SEISMICVELOCITYWYLLIE_H_
#define _SEISMICVELOCITYWYLLIE_H_

#include "SeismicVelocity.h"

namespace GeoPhysics
{
	/*! \class SeismicVelocityWyllie
	* \brief  Derived class of SeismicVelocity::Algorithm using the Wyllies's model for the seismic velocity computation.
	* \details \f[ Vp = \frac{ Vp_{fluid} * Vp_{solid} }{ \phi * Vp_{solid} + (1 - \phi)*Vp_{fluid} } \f]
	*/
	class SeismicVelocityWyllie : public SeismicVelocity::Algorithm
	{
	public:
		/*!
		* \param seismicVelocitySolid The seismic velocity of the matrix lithology (of the solid part, excluding porosity and fluid).
		*/
		SeismicVelocityWyllie(const double seismicVelocitySolid);

		virtual ~SeismicVelocityWyllie();

		/*!
		* \brief Compute the seismicVelocity of the bulk (inlc. prosity and fluid) using the Wyllies's model.
		* \param seismicVelocityFluid The seismic velocity of the fluid (-1 if there is no fluid).
		* \param densityFluid The fluid density (-1 if there is no fluid).
		* \param densityBulk The bulk density (inlc. prosity and fluid).
		* \param porosity The porosity.
		* \param currentVes The current vertical effective stress.
		* \param maxVes The maximum vertical effective stress.
		* \warning Only parameters seismicVelocityFluid and porosity are not used for the Wyllie computation mode.
		*/
		virtual double seismicVelocity(const double seismicVelocityFluid,
			const double densityFluid,
			const double densityBulk,
			const double porosity,
			const double currentVes,
			const double maxVes) const;

	private:
		/// The seismic velocity value of the matrix lithology (of the solid part, excluding porosity and fluid).
		const double m_seismicVelocitySolid;
	};
}

#endif