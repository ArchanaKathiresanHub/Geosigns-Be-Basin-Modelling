//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef _GEOPHYSICS__SOIL_MECHANICS_POROSITY_H_
#define _GEOPHYSICS__SOIL_MECHANICS_POROSITY_H_

#include "Porosity.h"

namespace GeoPhysics
{

   class soilMechanicsPorosity : public Porosity::Algorithm {
   public:
      soilMechanicsPorosity( const double depoPorosity,
                             const double minimumMechanicalPorosity,
                             const double soilMechanicsCompactionCoefficient,
                             const double depositionVoidRatio );

      /// Return porosity with soilMechanicsPorosity
      virtual double calculate( const double ves,
                                const double maxVes,
                                const bool includeChemicalCompaction,
                                const double chemicalCompactionTerm ) const;

      /// Return PorosityDerivative
      virtual double calculateDerivative( const double ves,
                                          const double maxVes,
                                          const bool includeChemicalCompaction,
                                          const double chemicalCompactionTerm ) const;

      /// \brief Determine if the porosity model is incompressible.
      virtual bool isIncompressible () const;

      /// Return the permeability model
      virtual Porosity::Model  model() const;

      /// Return Compaction coefficients 
      virtual double compactionCoefficient() const;
      
      /// Return Compaction coefficients 
      virtual double compactionCoefficientA() const { return 0.0; }
                  
      /// Return Compaction coefficients 
      virtual double compactionCoefficientB() const { return 0.0; }

   private:
      /// @brief Overwrite default assginment operator
      soilMechanicsPorosity& operator= (const soilMechanicsPorosity&);
      /// @brief Overwrite default copy constructor
      soilMechanicsPorosity( const soilMechanicsPorosity& );

      const double m_soilMechanicsCompactionCoefficient; ///< The soil mechanics compaction coefficient
      const double m_depositionVoidRatio;                ///< The depositional void ration
   };
}
#endif
