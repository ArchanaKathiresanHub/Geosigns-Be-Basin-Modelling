//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _GEOPHYSICS__SEISMICVELOCITY_H_
#define _GEOPHYSICS__SEISMICVELOCITY_H_

#include "GeoPhysicsFluidType.h"
#include "Interface.h"

#include "Porosity.h"

#include <memory>

namespace GeoPhysics
{
	/*! \class SeismicVelocity
	* \brief Abstract class defining the algorithm to compute the seismic velocity value.
	* The seismic velocity can be modelled with Gardner's, Kennan's or Wyllie's model.
	*/
	class SeismicVelocity
	{
	public:
		SeismicVelocity();
		typedef DataAccess::Interface::SeismicVelocityModel Model;

		/*!
		* \brief Create a seismicVelocity object corresponding to the seismic velocity model.
		* \param SeismicVelocityModel Model to compute seismic velocity. Can be Gardner's, Kennan's or Wyllie's.
		* \param seimsicVelocitySolid The seismic velocity value of the matrix lithology (of the solid part, excluding porosity and fluid).
		* \param modulusSolid The modulus value of the matrix lithology (of the solid part, excluding porosity and fluid).
		* \param densitySolid The density of the matrix lithology (of the solid part, excluding porosity and fluid).
		* \param porositySurface The depositional porosity (also called surface porosity or critical porosity).
		* \param nExponent The exponent used for the Kennan's velocity formula [-1,1]. In general, from pure shale (n=-1) to pure sand (n=1).
		*/
		static SeismicVelocity create(const Model SeismicVelocityModel,
			const double seimsicVelocitySolid,
			const double modulusSolid,
			const double densitySolid,
			const double porositySurface,
			const double nExponent);

		/*!
		* \brief Compute the seismicVelocity of the bulk (inlc. prosity and fluid) by calling its class member Algorithm.
		* \param seismicVelocityFluid The seismic velocity of the fluid (-1 if there is no fluid).
		* \param densityFluid The fluid density (-1 if there is no fluid).
		* \param densityBulk The bulk density (inlc. prosity and fluid).
		* \param porosity The porosity.
		* \param currentVes The current vertical effective stress.
		* \param maxVes The maximum vertical effective stress.
		*/
		double calculate( const double seismicVelocityFluid,
			const double densityFluid,
			const double densityBulk,
			const double porosity,
			const double currentVes,
			const double maxVes) const;

      /// @brief Overwrite default assginment operator to avoid bitwise copy
      SeismicVelocity& operator= (const SeismicVelocity& seismicVelocity);
      /// @brief Overwrite default copy constructor to avoid bitwise copy
      SeismicVelocity( const SeismicVelocity& seismicVelocity );

      ~SeismicVelocity();


		/*! \class Algorithm
		* \brief Abstract class member of seismicVelocity. Compute the seismicVelocity (of the bulk, inlc. prosity and fluid).
		*/
		class Algorithm
		{
		public:
			Algorithm();
			virtual ~Algorithm() {}

			/*!
			* \brief Compute the seismicVelocity of the bulk (inlc. prosity and fluid) via its child.
			* \param seismicVelocityFluid The seismic velocity of the fluid (-1 if there is no fluid).
			* \param densityFluid The fluid density (-1 if there is no fluid).
			* \param densityBulk The bulk density (inlc. prosity and fluid).
			* \param porosity The porosity.
			* \param currentVes The current vertical effective stress.
			* \param maxVes The maximum vertical effective stress.
			*/
				 virtual double calculate( const double seismicVelocityFluid,
				const double densityFluid,
				const double densityBulk,
				const double porosity,
				const double currentVes,
				const double maxVes) const = 0;
		};

	private:

		/// Private constructor setting the algorithm member.
		explicit SeismicVelocity(Algorithm * algorithm);

		/// SmartPointer to abstract class for seismic velocity computation.
		std::shared_ptr< Algorithm > m_algorithm;
	};

  inline double SeismicVelocity
      ::calculate( const double seismicVelocityFluid,
    const double densityFluid,
    const double densityBulk,
    const double porosity,
    const double currentVes,
    const double maxVes) const
  {
      return m_algorithm->calculate( seismicVelocityFluid,
        densityFluid,
        densityBulk,
        porosity,
        currentVes,
        maxVes);
  }
}

#endif
