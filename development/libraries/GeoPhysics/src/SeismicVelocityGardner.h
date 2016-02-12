//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _GEOPHYSICS__SEISMICVELOCITYGARDNER_H_
#define _GEOPHYSICS__SEISMICVELOCITYGARDNER_H_

#include "SeismicVelocity.h"


namespace GeoPhysics
{
	/*! \class SeismicVelocityGardner
	* \brief  Derived class of SeismicVelocity::Algorithm using the Garnder's model for the seismic velocity computation.
   * It is refered in the UI as the "Density Power Law (Gardner)" seismic velocity algorithm.
	* \details \f[ Vp = (\frac{ \rho_{bulk} }{ G })^4 \f] where \f[G = 309.4\f]
	*	G is the Gardner constant.
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
		* \param currentVes The current vertical effective stress.
		* \param maxVes The maximum vertical effective stress.
		* \warning Only densityBulk parameter is used for the Gardner computation mode.
		*/
      virtual double calculate( const double seismicVelocityFluid,
			const double densityFluid,
			const double densityBulk,
			const double porosity,
			const double currentVes,
			const double maxVes) const;

   private:
      /// @brief Overwrite default assginment operator
      SeismicVelocityGardner& operator= (const SeismicVelocityGardner&);
      /// @brief Overwrite default copy constructor
      SeismicVelocityGardner( const SeismicVelocityGardner& );
	};
}

#endif