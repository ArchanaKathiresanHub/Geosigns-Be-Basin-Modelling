//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef _GEOPHYSICS__PERMEABILITYSANDSTONE_H_
#define _GEOPHYSICS__PERMEABILITYSANDSTONE_H_

#include "ArrayDefinitions.h"
#include "Permeability.h"

namespace GeoPhysics
{

   class PermeabilitySandStone: public Permeability::Algorithm
   {
   public:
      PermeabilitySandStone( double depoPorosity, double depoPermeability, double permeabilityIncr);

      /// @brief Compte the sandstone model permeability
      virtual double calculate ( const double ves,
                                 const double maxVes,
                                 const double calculatedPorosity ) const;

      /// \brief Compute the permeability for the sandstone model for an array of values
      virtual void calculate ( const unsigned int       n,
                               ArrayDefs::ConstReal_ptr ves,
                               ArrayDefs::ConstReal_ptr maxVes,
                               ArrayDefs::ConstReal_ptr calculatedPorosity,
                               ArrayDefs::Real_ptr      permeabilities ) const;

      /// \brief Compte the derivative of the sandstone permeability function with respect to ves.
      virtual void calculateDerivative ( const double  ves,
                                         const double  maxVes,
                                         const double  calculatedPorosity,
                                         const double  porosityDerivativeWrtVes,
                                               double& permeability,
                                               double& derivative ) const;

      /// \brief Compute the permeability and its derivative for the sandstone model for an array of values
      virtual void calculateDerivative ( const unsigned int       n,
                                         ArrayDefs::ConstReal_ptr ves,
                                         ArrayDefs::ConstReal_ptr maxVes,
                                         ArrayDefs::ConstReal_ptr calculatedPorosity,
                                         ArrayDefs::ConstReal_ptr porosityDerivativeWrtVes,
                                         ArrayDefs::Real_ptr      permeabilities,
                                         ArrayDefs::Real_ptr      derivatives ) const;

      /// return the depositional permeability
      virtual double depoPerm() const ;

      /// Return the permeability model
      virtual Permeability::Model  model() const ;

   private:

      double calculateSingleValue ( const double calculatedPorosity ) const;

      /// @brief Overwrite default assginment operator
      PermeabilitySandStone& operator= (const PermeabilitySandStone&);
      /// @brief Overwrite default copy constructor
      PermeabilitySandStone( const PermeabilitySandStone& );

      const double m_depoPorosity;      ///< The depositional porosity
      const double m_depoPermeability;  ///< the depositional permeability
      const double m_permeabilityIncr;

      /// \brief Pre-compute 100.0 * Log10 * ( 0.12 + 0.02 * permeabilityIncr )
      const double m_term;

   };
}

#endif
