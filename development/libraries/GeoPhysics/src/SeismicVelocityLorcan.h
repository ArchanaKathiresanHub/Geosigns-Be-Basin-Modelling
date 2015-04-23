#ifndef _SEISMICVELOCITYLORCAN_H_
#define _SEISMICVELOCITYLORCAN_H_

#include "SeismicVelocity.h"

namespace GeoPhysics
{
	/*! \class SeismicVelocityLorcan
	* \brief  Derived class of SeismicVelocity::Algorithm using the Lorcan's model for the seismic velocity computation.
	*/
	class SeismicVelocityLorcan : public SeismicVelocity::Algorithm
	{
	public:
		/*!
		* \param seimsicVelocitySolid The seismic velocity value of the matrix lithology (of the solid part, excluding porosity and fluid).
		* \param densitySolid The density of the matrix lithology (of the solid part, excluding porosity and fluid).
		* \param prositySurface The depositional porosity (also called surface porosity or critical porosity).
		* \param nExponent The exponent used for the Lorcan's velocity formula [-1,1]. In general, from pure shale (n=-1) to pure sand (n=1).
		*/
		SeismicVelocityLorcan(const double seimsicVelocitySolid,
			const double densitySolid,
			const double porositySurface,
			const double nExponent);

		virtual ~SeismicVelocityLorcan();

		/*!
		* \brief Compute the seismicVelocity of the bulk (inlc. prosity and fluid) using the Lorcan's model.
		* \param seismicVelocityFluid The seismic velocity of the fluid (-1 if there is no fluid).
		* \param densityFluid The fluid density (-1 if there is no fluid).
		* \param densityBulk The bulk density (inlc. prosity and fluid).
		* \param porosity The porosity.
		*/
		virtual double seismicVelocity(const double seismicVelocityFluid,
			const double densityFluid,
			const double densityBulk,
			const double porosity) const;

	private:
		/// The seismic velocity value of the matrix lithology (of the solid part, excluding porosity and fluid).
		const double m_seimsicVelocitySolid;
		/// The density of the matrix lithology (of the solid part, excluding porosity and fluid).
		const double m_densitySolid;
		/// The depositional porosity (also called surface porosity or critical porosity).
		const double m_porositySurface;
		/// The exponent used for the Lorcan's velocity formula [-1,1]. In general, from pure shale (n=-1) to pure sand (n=1).
		const double m_nExponent;
	};
}

#endif