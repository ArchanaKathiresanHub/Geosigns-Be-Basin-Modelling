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
		* \param SeismicVelocityModel Model to compute seismic velocity. Can be Gardner's or Wyllie's.
		* \param seimsicVelocitySolid The seismic velocity value of the lithology (of the solid part, excluding porosity and fluid).
		*/
		SeismicVelocity create(const Model SeismicVelocityModel,
			const double seimsicVelocitySolid) const;

		/*!
		* \brief Compute the seismicVelocity (of the bulk, inlc. prosity and fluid) by calling its class member Algorithm.
		* \param fluid The fluid (must be a NULL pointer if there is no fluid).
		* \param density The bulk density.
		* \param porePressure The pore pressure.
		* \param temperature The temperature.
		*/
		double seismicVelocity(const FluidType* fluid,
			const double density,
			const double porosity,
			const double porePressure,
			const double temperature) const;

		/*! \class Algorithm
		* \brief Abstract class member of seismicVelocity. Compute the seismicVelocity (of the bulk, inlc. prosity and fluid).
		*/
		class Algorithm
		{
		public:
			Algorithm();
			virtual ~Algorithm() {}

			/*!
			* \brief Compute the seismicVelocity (of the bulk, inlc. prosity and fluid) via its child.
			* \param fluid The fluid (must be a NULL pointer if there is no fluid).
			* \param density The bulk density.
			* \param porePressure The pore pressure.
			* \param temperature The temperature.
			*/
			virtual double seismicVelocity(const FluidType* fluid,
				const double density,
				const double porosity,
				const double porePressure,
				const double temperature) const = 0;
		};

	private:
		/// Private constructor setting the algorithm member.
		SeismicVelocity(Algorithm * algorithm);

		/// SmartPointer to abstract class for seismic velocity computation.
		boost::shared_ptr< Algorithm > m_algorithm;
	};

	inline double SeismicVelocity
		::seismicVelocity(const FluidType* fluid,
		const double density,
		const double porosity,
		const double porePressure,
		const double temperature) const
	{
			return m_algorithm->seismicVelocity(fluid, density, porosity, porePressure, temperature);
	}
}

#endif