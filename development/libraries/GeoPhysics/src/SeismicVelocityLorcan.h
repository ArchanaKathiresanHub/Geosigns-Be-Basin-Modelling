#ifndef _GEOPHYSICS__SEISMICVELOCITYLORCAN_H_
#define _GEOPHYSICS__SEISMICVELOCITYLORCAN_H_

#include "SeismicVelocity.h"

namespace GeoPhysics
{
	/*! \class SeismicVelocityLorcan
	* \brief  Derived class of SeismicVelocity::Algorithm using the Lorcan's model for the seismic velocity computation.
	* \details
	*
	*   If VES=MaxVES, we are on the compaction way
	*	\f[ M_{solid} = \rho_{solid} * Vp_{solid}^2 \f]
	*	\f[ M_{fluid} = \rho_{fluid}(P,T) * Vp_{fluid}(P,T)^2 \f]
	*	\f[ M_{surface} = \frac{M_{solid} * Vp_{fluid}}{\phi_{surface} * M_{solid} + (1-\phi_{surface}) * M_{fluid}} \f]
	*	\f[ M = M_{surface}^{\frac{\phi}{\phi_{surface}}} * M_{solid}^{1-\frac{\phi}{\phi_{surface}}} : n = 0 \f]
	*	\f[ M = (M_{surface}^n*\frac{\phi}{\phi_{surface}} * M_{solid}^n*(1-\frac{\phi}{\phi_{surface}}))^{\frac{1}{n}} : n \ne 0 \f]
	*   \f[ Vp = \sqrt{\frac{M}{\rho_{bulk}}} \f]
	*
	*   If VES<MaxVES, we are on the decompaction way
	*   \f[ M_{Reuss} = (M_{surface}^{-1}*\frac{\phi}{\phi_{surface}} * M_{solid}^{-1}*(1-\frac{\phi}{\phi_{surface}}))^{-1} \f]
	*   \f[ M = M_{VES=MaxVES} - \frac{\phi}{\phi_{surface}} * \frac{MaxVES-VES}{MaxVES} * (M_{VES=MaxVES} - M_{Reuss}) \f]
	*   \f[ Vp = \sqrt{\frac{M}{\rho_{bulk}}} \f]
	*/
	class SeismicVelocityLorcan : public SeismicVelocity::Algorithm
	{
	public:
		/*!
		* \param modulusSolid The modulus value of the matrix lithology (of the solid part, excluding porosity and fluid).
		* \param densitySolid The density of the matrix lithology (of the solid part, excluding porosity and fluid).
		* \param porositySurface The depositional porosity (also called surface porosity or critical porosity).
		* \param nExponent The exponent used for the Lorcan's velocity formula [-1,1]. In general, from pure shale (n=-1) to pure sand (n=1).
		*/
		SeismicVelocityLorcan(const double modulusSolid,
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
		* \param currentVes The current vertical effective stress.
		* \param maxVes The maximum vertical effective stress.
		*/
		virtual double seismicVelocity(const double seismicVelocityFluid,
			const double densityFluid,
			const double densityBulk,
			const double porosity,
			const double currentVes,
			const double maxVes) const;

	private:

		/*!
		* \brief Return the bulk modulus at a defined porosity.
		* \param modulusSurface The surface (also called critical) modulus, this is the bulk modulus at the surface porosity.
		* \param porosityScaled The current porosity divided by the surface porosity.
		* \param nExponent The exponent used for the Lorcan's velocity formula [-1,1]. In general, from pure shale (n=-1) to pure sand (n=1).
		*/
		double modulusN(const double modulusSurface,
			const double porosityScaled,
			const double nExponent) const;

		/// The seismic velocity value of the matrix lithology (of the solid part, excluding porosity and fluid).
		const double m_modulusSolid;
		/// The density of the matrix lithology (of the solid part, excluding porosity and fluid).
		const double m_densitySolid;
		/// The depositional porosity (also called surface porosity or critical porosity).
		const double m_porositySurface;
		/// The exponent used for the Lorcan's velocity formula [-1,1]. In general, from pure shale (n=-1) to pure sand (n=1).
		const double m_nExponent;
	};
}

#endif