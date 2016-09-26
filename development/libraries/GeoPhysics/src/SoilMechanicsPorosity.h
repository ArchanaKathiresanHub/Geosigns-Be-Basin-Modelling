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

#include "ArrayDefinitions.h"
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
      virtual Porosity::Model  model() const;

      /// Return Compaction coefficients
      virtual double compactionCoefficient() const;

      /// Return Compaction coefficients
      virtual double compactionCoefficientA() const;

      /// Return Compaction coefficients
      virtual double compactionCoefficientB() const;

   private:
      /// @brief Overwrite default assginment operator
      soilMechanicsPorosity& operator= (const soilMechanicsPorosity&);
      /// @brief Overwrite default copy constructor
      soilMechanicsPorosity( const soilMechanicsPorosity& );

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


      const double m_soilMechanicsCompactionCoefficient; ///< The soil mechanics compaction coefficient
      const double m_depositionVoidRatio;                ///< The depositional void ration
      const double m_percentagePorosityRebound;          ///< Age porosity regain
   };


   inline double
   soilMechanicsPorosity::calculate( const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm ) const
   {
      return computeSingleValue( ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm );
   }


   inline double
   soilMechanicsPorosity::calculateDerivative(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const
   {
      return computeSingleValueDerivative( computeSingleValue(ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm), ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm );
   }


   inline bool
   soilMechanicsPorosity::isIncompressible() const
   {
      return m_soilMechanicsCompactionCoefficient == 0.0;
   }


   inline Porosity::Model
   soilMechanicsPorosity::model() const
   {
      return DataAccess::Interface::SOIL_MECHANICS_POROSITY;
   }


   inline double
   soilMechanicsPorosity::compactionCoefficient() const
   {
      return m_soilMechanicsCompactionCoefficient;
   }


   inline double
   soilMechanicsPorosity::compactionCoefficientA () const
   {
      return 0.0;
   }


   inline double
   soilMechanicsPorosity::compactionCoefficientB () const
   {
      return 0.0;
   }
}
#endif
