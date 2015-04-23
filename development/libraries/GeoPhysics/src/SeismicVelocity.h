#ifndef _SEISMICVELOCITY_H_
#define _SEISMICVELOCITY_H_

#include <boost/shared_ptr.hpp>
#include "GeoPhysicsFluidType.h"
#include "Interface/Interface.h"

namespace GeoPhysics
{
	/*! \class SeismicVelocity
	* \brief Abstract class defining the algorithm to compute the seismic velocity value.
	* The seismic velocity can be modelled with Gardner's or Wyllie's model.
	*/
	class SeismicVelocity
	{
	public:
		SeismicVelocity();
		typedef DataAccess::Interface::SeismicVelocityModel Model;

		/*!
		* \brief Create a seismicVelocity object corresponding to the seismic velocity model.
		* \param SeismicVelocityModel Model to compute seismic velocity. Can be Gardner's, Lorcan's or Wyllie's.
		* \param seimsicVelocitySolid The seismic velocity value of the matrix lithology (of the solid part, excluding porosity and fluid).
		* \param densitySolid The density of the matrix lithology (of the solid part, excluding porosity and fluid).
		* \param prositySurface The depositional porosity (also called surface porosity or critical porosity).
		* \param nExponent The exponent used for the Lorcan's velocity formula [-1,1]. In general, from pure shale (n=-1) to pure sand (n=1).
		*/
		SeismicVelocity create(const Model SeismicVelocityModel,
			const double seimsicVelocitySolid,
			const double densitySolid,
			const double porositySurface,
			const double nExponent) const;

		/*!
		* \brief Compute the seismicVelocity of the bulk (inlc. prosity and fluid) by calling its class member Algorithm.
		* \param seismicVelocityFluid The seismic velocity of the fluid (-1 if there is no fluid).
		* \param densityFluid The fluid density (-1 if there is no fluid).
		* \param densityBulk The bulk density (inlc. prosity and fluid).
		* \param porosity The porosity.
		*/
		double seismicVelocity(const double seismicVelocityFluid,
			const double densityFluid,
			const double densityBulk,
			const double porosity) const;

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
			*/
			virtual double seismicVelocity(const double seismicVelocityFluid,
				const double densityFluid,
				const double densityBulk,
				const double porosity) const = 0;
		};

	private:
		/// Private constructor setting the algorithm member.
		SeismicVelocity(Algorithm * algorithm);

		/// SmartPointer to abstract class for seismic velocity computation.
		boost::shared_ptr< Algorithm > m_algorithm;
	};

	inline double SeismicVelocity
		::seismicVelocity(const double seismicVelocityFluid,
		const double densityFluid,
		const double densityBulk,
		const double porosity) const
	{
			return m_algorithm->seismicVelocity(seismicVelocityFluid,
				densityFluid,
				densityBulk,
				porosity);
	}
}

#endif