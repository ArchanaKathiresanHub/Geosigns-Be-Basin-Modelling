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

#include "ArrayDefinitions.h"
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
                                 const double compactionDecrB,
                                 const bool   isLegacy );

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

      /// Return porosity (vectorized version)
      /// \pre arrays MUST BE ALIGNED
      virtual void calculate( const unsigned int n,
                              ArrayDefs::ConstReal_ptr ves,
                              ArrayDefs::ConstReal_ptr maxVes,
                              const bool includeChemicalCompaction,
                              ArrayDefs::ConstReal_ptr chemicalCompactionTerm,
                              ArrayDefs::Real_ptr porosities ) const;

      /// Return porosity and its derivative (vectorized version)
      /// \pre arrays MUST BE ALIGNED
      virtual void calculate( const unsigned int n,
                              ArrayDefs::ConstReal_ptr ves,
                              ArrayDefs::ConstReal_ptr maxVes,
                              const bool includeChemicalCompaction,
                              ArrayDefs::ConstReal_ptr chemicalCompactionTerm,
                              ArrayDefs::Real_ptr porosities,
                              ArrayDefs::Real_ptr porosityDers ) const;

      /// \brief Determine if the porosity model is incompressible.
      virtual bool isIncompressible () const;

      /// Return the permeability model
      virtual Porosity::Model model() const;

      /// Return Compaction coefficients (only m_compactionincr for exponential)
      virtual double compactionCoefficient() const;
      virtual double compactionCoefficientA() const;
      virtual double compactionCoefficientB() const;

   private:
      /// @brief Overwrite default assginment operator
      DoubleExponentialPorosity& operator= (const DoubleExponentialPorosity&);
      /// @brief Overwrite default copy constructor
      DoubleExponentialPorosity( const DoubleExponentialPorosity& );

      /// @brief Compute a single porosity value
      double computeSingleValue( const double ves,
                                 const double maxVes,
                                 const bool includeChemicalCompaction,
                                 const double chemicalCompactionTerm ) const;

      /// @brief Compute a single porosity derivative value
      double computeSingleValueDerivative( const double porosity,
                                           const double ves,
                                           const double maxVes,
                                           const bool includeChemicalCompaction,
                                           const double chemicalCompactionTerm ) const;


      const double m_compactionIncrA; ///< The first member loading phase compaction coefficient
      const double m_compactionIncrB; ///< The seconf member loading phase compaction coefficient
      const double m_compactionDecrA; ///< The first member unloading phase compaction coefficient
      const double m_compactionDecrB; ///< The seconf member unloading phase compaction coefficient
   };


   inline double
   DoubleExponentialPorosity::calculate( const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm ) const
   {
      return computeSingleValue( ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm );
   }


   inline double
   DoubleExponentialPorosity::calculateDerivative(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const
   {
      return computeSingleValueDerivative( computeSingleValue(ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm), ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm );
   }


   inline bool
   DoubleExponentialPorosity::isIncompressible () const
   {
      return (m_compactionIncrA == 0.0) and (m_compactionIncrB == 0.0);
   }


   inline Porosity::Model
   DoubleExponentialPorosity::model () const
   {
      return DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY;
   }


   inline double
   DoubleExponentialPorosity::compactionCoefficient () const
   {
      return 0.0;
   }


   inline double
   DoubleExponentialPorosity::compactionCoefficientA () const
   {
      return m_compactionIncrA;
   }


   inline double
   DoubleExponentialPorosity::compactionCoefficientB () const
   {
      return m_compactionIncrB;
   }
}
#endif
