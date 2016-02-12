//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef _GEOPHYSICS__EXPONENTIAL_POROSITY_H_
#define _GEOPHYSICS__EXPONENTIAL_POROSITY_H_

#include "Porosity.h"



namespace GeoPhysics
{

   class ExponentialPorosity : public Porosity::Algorithm {
   public:
      ExponentialPorosity(double depoPorosity,
    	                    double minimumMechanicalPorosity,
                          double compactionIncr,
                          double compactionDecr);

      /// Return porosity with exponential function
      virtual double calculate(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const;

      /// Return PorosityDerivative
      virtual double calculateDerivative( const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm ) const;

      /// \brief Determine if the porosity model is incompressible.
      virtual bool isIncompressible () const;

      /// Return the permeability model
      virtual Porosity::Model  model() const;

      /// Return FullCompThickness
      virtual double fullCompThickness(const double MaxVesValue, const double thickness, const double densitydiff, const double vesScaleFactor, const bool overpressuredCompaction) const;

      /// Return Compaction coefficients
      virtual double compactionCoefficient() const;
      
      /// Return Compaction coefficients 
      virtual double compactionCoefficientA() const { return 0.0; }

      /// Return Compaction coefficients 
      virtual double compactionCoefficientB() const { return 0.0; }
      

   private:
      /// @brief Overwrite default assginment operator
      ExponentialPorosity& operator= (const ExponentialPorosity&);
      /// @brief Overwrite default copy constructor
      ExponentialPorosity( const ExponentialPorosity& );

      const double  m_compactionIncr; ///< The loading phase compaction coefficient
      const double  m_compactionDecr; ///< The unloading phase compaction coefficient
   };
}
#endif
