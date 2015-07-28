//                                                                      
// Copyright (C) 2015-2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SeismicVelocityKennan.h"
#include <cmath>

namespace GeoPhysics
{
	SeismicVelocityKennan::SeismicVelocityKennan(const double modulusSolid,
		const double densitySolid,
		const double porositySurface,
		const double nExponent) :
		Algorithm(),
		m_modulusSolid(modulusSolid),
		m_densitySolid(densitySolid),
		m_porositySurface(porositySurface),
		m_nExponent(nExponent)
	{}

	SeismicVelocityKennan::~SeismicVelocityKennan()
	{}

	double SeismicVelocityKennan::seismicVelocity(const double seismicVelocityFluid,
		const double densityFluid,
		const double densityBulk,
		const double porosity,
		const double currentVes,
		const double maxVes) const
	{
		double velocity = 0;
		/// If there is a fluid
      if (seismicVelocityFluid != -1 && densityFluid != -1 && m_porositySurface != 0)
		{
			///1 Compute fluid modulus
			double modulusFluid = densityFluid*seismicVelocityFluid*seismicVelocityFluid;

			///2 Compute the surface modulus
			double denominator = modulusFluid*(1 - m_porositySurface) + m_modulusSolid*(m_porositySurface);
			assert(denominator!=0);
			double modulusSurface = (modulusFluid*m_modulusSolid) / denominator;

			///3 Scale the porosity for the modulus computation
			double porosityScaled = porosity / m_porositySurface;
			if (porosityScaled >= 1){
				//suspension line (phi>phi_surf, so this is no more a solid rock)
				return seismicVelocityFluid;
			}

			///4 Compute the prograde modulus
			double modulusPrograde = this->modulusN(modulusSurface, porosityScaled, m_nExponent);

			double modulusN = 0;
			///5 If we are in retrograde path (decompaction)
			if (currentVes < maxVes)
			{
				///5.1. Compute the Reuss modulus (n=-1)
				double modulusReuss = this->modulusN(modulusSurface, porosityScaled, -1);

				///5.2. Compute the Delta modulus between prograde and Reuss
				double modulusDelta = modulusPrograde - modulusReuss;

				///5.3. Compute the modulus as the prograde modulus minus a fraction of the delta modulus
				modulusN = modulusPrograde - (maxVes - currentVes) / maxVes * porosityScaled * modulusDelta;
			}
			///5 If we are one the prograde path (compaction)
			else
			{
				///5.1. Compute the modulus at the defined porosity at 'n' exponent
				modulusN = modulusPrograde;
			}

			///6 Compute the seismic velocity
			velocity = sqrt(modulusN / densityBulk);

		}

		/// Else there is no fluid, the velocity equal the solid velocity
		else
		{
			velocity = sqrt(m_modulusSolid / m_densitySolid);
		}

		return velocity;
	}

	double SeismicVelocityKennan::modulusN(const double modulusSurface,
		const double porosityScaled,
		const double nExponent) const
	{
		double modulusN = 0;

		if (nExponent == 0){
			// function aproximation
			modulusN = std::pow(modulusSurface, porosityScaled)*std::pow(m_modulusSolid, (1 - porosityScaled));
		}
		else{
			modulusN = porosityScaled*std::pow(modulusSurface, nExponent) + (1 - porosityScaled)*std::pow(m_modulusSolid, nExponent);
			modulusN = std::pow(modulusN, (1 / nExponent));
		}

		return modulusN;
	}
}