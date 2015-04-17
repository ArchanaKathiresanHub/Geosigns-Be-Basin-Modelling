#ifndef _SEISMICVELOCITY_H_
#define _SEISMICVELOCITY_H_

#include <iostream>
using namespace std;
#include <boost/shared_ptr.hpp>
#include <string>

#include "GeoPhysicsFluidType.h"
#include "Interface/Interface.h"

namespace GeoPhysics
{

	class SeismicVelocity
	{
	public:
		SeismicVelocity();
		typedef DataAccess::Interface::SeismicVelocityModel Model;
		//virtual ~SeismicVelocity();

		SeismicVelocity create(const Model SeismicVelocityModel,
			const double seimsicVelocitySolid);

		double seismicVelocity(const FluidType* fluid,
			const double density,
			const double porosity,
			const double porePressure,
			const double temperature) const;

		Model getSeismicVelocityModel() const;

		class Algorithm
		{
		public:
			Algorithm();
			virtual ~Algorithm() {}

			virtual double seismicVelocity(const FluidType* fluid,
				const double density,
				const double porosity,
				const double porePressure,
				const double temperature) const = 0;

			virtual Model model() const = 0;
		};

	private:
		SeismicVelocity(Algorithm * algorithm);

		boost::shared_ptr< Algorithm > m_algorithm;
	};

	inline DataAccess::Interface::SeismicVelocityModel
		SeismicVelocity::getSeismicVelocityModel() const
	{
		return m_algorithm->model();
	}

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