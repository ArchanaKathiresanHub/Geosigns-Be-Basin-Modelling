//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SeismicVelocity.h"
#include "SeismicVelocityGardner.h"
#include "SeismicVelocityKennan.h"
#include "SeismicVelocityWyllie.h"

namespace GeoPhysics
{

	SeismicVelocity::SeismicVelocity()
      :m_algorithm()
	{}

	SeismicVelocity::SeismicVelocity(Algorithm * algorithm)
		:m_algorithm(algorithm)
	{}

	SeismicVelocity SeismicVelocity::create(const Model SeismicVelocityModel,
		const double seimsicVelocitySolid,
		const double modulusSolid,
		const double densitySolid,
		const double porositySurface,
		const double nExponent)
	{

		switch (SeismicVelocityModel)
		{
		case DataAccess::Interface::GARDNERS_VELOCITY_ALGORITHM:
			return SeismicVelocity(new SeismicVelocityGardner());

		case DataAccess::Interface::KENNANS_VELOCITY_ALGORITHM:
			return SeismicVelocity(new SeismicVelocityKennan(modulusSolid,
				densitySolid,
				porositySurface,
				nExponent));

		case DataAccess::Interface::WYLLIES_VELOCITY_ALGORITHM:
			return SeismicVelocity(new SeismicVelocityWyllie(seimsicVelocitySolid));
		}

		return SeismicVelocity(0);
	}

   SeismicVelocity& SeismicVelocity::operator= (const SeismicVelocity& seismicVelocity){
      if (this != &seismicVelocity) {
         m_algorithm = seismicVelocity.m_algorithm;
      }
      return *this;
   }

   SeismicVelocity::SeismicVelocity( const SeismicVelocity& seismicVelocity )
     :m_algorithm( seismicVelocity.m_algorithm )
   {}

   SeismicVelocity::~SeismicVelocity()
   {
   }

	SeismicVelocity::Algorithm::Algorithm()
	{}

}
