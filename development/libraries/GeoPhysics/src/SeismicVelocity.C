#include "SeismicVelocity.h"
#include "SeismicVelocityGardner.h"

namespace GeoPhysics
{

	SeismicVelocity::SeismicVelocity():m_algorithm()
	{}

	SeismicVelocity::SeismicVelocity(Algorithm * algorithm)
		:m_algorithm(algorithm)
	{}

	SeismicVelocity SeismicVelocity::create(const Model SeismicVelocityModel,
		const double seimsicVelocitySolid)
	{

		switch (SeismicVelocityModel)
		{
		case DataAccess::Interface::GARDNERS_VELOCITY_ALGORITHM:
			return SeismicVelocity(new SeismicVelocityGardner());
		}

		return SeismicVelocity(0);
	}

	SeismicVelocity::Algorithm::Algorithm()
	{}

}