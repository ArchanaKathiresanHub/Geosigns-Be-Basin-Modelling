#include "SeismicVelocityLorcan.h"

namespace GeoPhysics
{
	SeismicVelocityLorcan::SeismicVelocityLorcan(const double seimsicVelocitySolid,
		const double densitySolid,
		const double porositySurface,
		const double nExponent) :
		Algorithm(),
		m_seimsicVelocitySolid(seimsicVelocitySolid),
		m_densitySolid(densitySolid),
		m_porositySurface(porositySurface),
		m_nExponent(nExponent)
	{}

	SeismicVelocityLorcan::~SeismicVelocityLorcan()
	{}

	double SeismicVelocityLorcan::seismicVelocity(const double seismicVelocityFluid,
		const double densityFluid,
		const double densityBulk,
		const double porosity) const
	{
		double velocity = 0;
		//If there is a fluid
		if (seismicVelocityFluid != -1){
	    	///1. Compute fluid and solid modulus
			double modulusFluid = densityFluid*seismicVelocityFluid*seismicVelocityFluid;
			double modulusSolid = m_densitySolid*m_seimsicVelocitySolid*m_seimsicVelocitySolid;

			///2. Compute the surface modulus
			double denominator = modulusFluid*(1 - m_porositySurface) + modulusSolid*(m_porositySurface);
			assert(denominator!=0);
			double modulusSurface = (modulusFluid*modulusSolid) / denominator;

			///3. Scale the porosity
			double porosityScaled = porosity / m_porositySurface;
			if (porosityScaled >= 1){
				//suspension line
				velocity = seismicVelocityFluid;
			}

			else{

				///4. Compute the modulus at the defined porosity an 'n' exponent
				double modulusN = 0;
				if (m_nExponent == 0){
					// *** aproximation
					modulusN = pow(modulusSurface, porosityScaled)*pow(modulusSolid, (1 - porosityScaled));
				}
				else{
					modulusN = porosityScaled*pow(modulusSurface, m_nExponent) + (1 - porosityScaled)*pow(modulusSolid, m_nExponent);
					modulusN = pow(modulusN, (1 / m_nExponent));
				}

				///5. Compute the seismic velocity
				velocity = sqrt(modulusN / densityBulk);
			}

	    }
	    //Else, if there is no fluid
	    else {
			velocity = m_seimsicVelocitySolid;
	    }
	    return velocity;
	}
}