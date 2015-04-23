#include "SeismicVelocity.h"
#include "SeismicVelocityGardner.h"
#include "SeismicVelocityLorcan.h"
#include "SeismicVelocityWyllie.h"

namespace GeoPhysics
{

	SeismicVelocity::SeismicVelocity():m_algorithm()
	{}

	SeismicVelocity::SeismicVelocity(Algorithm * algorithm)
		:m_algorithm(algorithm)
	{}

	SeismicVelocity SeismicVelocity::create(const Model SeismicVelocityModel,
		const double seimsicVelocitySolid,
		const double densitySolid,
		const double porositySurface,
		const double nExponent) const
	{

		switch (SeismicVelocityModel)
		{
		case DataAccess::Interface::GARDNERS_VELOCITY_ALGORITHM:
			return SeismicVelocity(new SeismicVelocityGardner());

		case DataAccess::Interface::LORCANS_VELOCITY_ALGORITHM:
			return SeismicVelocity(new SeismicVelocityLorcan(seimsicVelocitySolid,
				densitySolid,
				porositySurface,
				nExponent));

		case DataAccess::Interface::WYLLIES_VELOCITY_ALGORITHM:
			return SeismicVelocity(new SeismicVelocityWyllie(seimsicVelocitySolid));
		}

		return SeismicVelocity(0);
	}

	SeismicVelocity::Algorithm::Algorithm()
	{}

}