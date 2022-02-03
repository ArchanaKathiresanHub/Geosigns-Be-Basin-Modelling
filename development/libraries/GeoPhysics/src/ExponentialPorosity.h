//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef _GEOPHYSICS__EXPONENTIAL_POROSITY_H_
#define _GEOPHYSICS__EXPONENTIAL_POROSITY_H_

#include "ArrayDefinitions.h"
#include "Porosity.h"

namespace GeoPhysics
{

   class ExponentialPorosity : public Porosity::Algorithm {
   public:
      ExponentialPorosity( const double depoPorosity,
                           const double minimumMechanicalPorosity,
                           const double compactionIncr,
                           const double compactionDecr,
                           const bool   isLegacy);

      /// Return porosity with exponential function
      virtual double calculate( const double ves,
                                const double maxVes,
                                const bool includeChemicalCompaction,
                                const double chemicalCompactionTerm) const;

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

      /// Return FullCompThickness
      virtual double fullCompThickness( const double maxVesValue,
                                        const double thickness,
                                        const double densitydiff,
                                        const double vesScaleFactor,
                                        const bool overpressuredCompaction) const;

      /// Return Compaction coefficients
      virtual double compactionCoefficient() const;

      /// Return Compaction coefficients
      virtual double compactionCoefficientA() const;

      /// Return Compaction coefficients
      virtual double compactionCoefficientB() const;

      /// Return Compaction ratio
      virtual double compactionRatio() const;

   private:
      /// @brief Overwrite default assignment operator
      ExponentialPorosity& operator= (const ExponentialPorosity&);
      /// @brief Overwrite default copy constructor
      ExponentialPorosity( const ExponentialPorosity& );

      /// @brief Compute a single porosity value
      double computeSingleValue( const double ves,
                                 const double maxVes,
                                 const bool includeChemicalCompaction,
                                 const double chemicalCompactionTerm ) const;

      /// @brief Compute a single porosity value
      double computeSingleValueDerivative( const double porosity,
                                           const double ves,
                                           const double maxVes,
                                           const bool includeChemicalCompaction,
                                           const double chemicalCompactionTerm ) const;
      ///< The loading phase compaction coefficient
      const double m_compactionIncr; 
      ///< The unloading phase compaction coefficient
      const double m_compactionDecr; 
   };


   inline double
   ExponentialPorosity::calculate( const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm ) const
   {
      return computeSingleValue( ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm );
   }


   inline double
   ExponentialPorosity::calculateDerivative(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const
   {
      return computeSingleValueDerivative( computeSingleValue(ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm), ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm );
   }


   inline bool
   ExponentialPorosity::isIncompressible () const
   {
      return m_compactionIncr == 0.0;
   }


   inline Porosity::Model
   ExponentialPorosity::model () const
   {
      return DataAccess::Interface::EXPONENTIAL_POROSITY;
   }


   inline double
    ExponentialPorosity::compactionCoefficient () const
   {
      return m_compactionIncr;
   }


   inline double
   ExponentialPorosity::compactionCoefficientA () const
   {
      return 0.0;
   }


   inline double
   ExponentialPorosity::compactionCoefficientB () const
   {
      return 0.0;
   }

   inline double
      ExponentialPorosity::compactionRatio() const
   {
      return 0.0;
   }

}

#endif
