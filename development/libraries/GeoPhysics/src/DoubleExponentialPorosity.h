//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef _GEOPHYSICS__DOUBLE_EXPONENTIAL_POROSITY_H_
#define _GEOPHYSICS__DOUBLE_EXPONENTIAL_POROSITY_H_

#include "Porosity.h"



namespace GeoPhysics
{

   class DoubleExponentialPorosity : public Porosity::Algorithm {
   public:
      DoubleExponentialPorosity( const double depoPorosity,
                                 const double minimumMechanicalPorosity,
                                 const double compactionIncrA, 
                                 const double compactionIncrB,
                                 const double compactionDecrA,
                                 const double compactionDecrB );

      /// Return porosity with exponential function
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

      /// Return Compaction coefficients (only m_compactionincr for exponential)
      virtual double compactionCoefficient() const{ return 0.0;}
      virtual double compactionCoefficientA() const;
      virtual double compactionCoefficientB() const;

   private:
      /// @brief Overwrite default assginment operator
      DoubleExponentialPorosity& operator= (const DoubleExponentialPorosity&);
      /// @brief Overwrite default copy constructor
      DoubleExponentialPorosity( const DoubleExponentialPorosity& );
      
      const double m_compactionIncrA; ///< The first member loading phase compaction coefficient
      const double m_compactionIncrB; ///< The seconf member loading phase compaction coefficient
      const double m_compactionDecrA; ///< The first member unloading phase compaction coefficient
      const double m_compactionDecrB; ///< The seconf member unloading phase compaction coefficient
   };
}
#endif
