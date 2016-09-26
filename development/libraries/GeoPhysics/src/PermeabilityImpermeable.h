//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef _GEOPHYSICS__PERMEABILITYIMPERMEABLE_H_
#define _GEOPHYSICS__PERMEABILITYIMPERMEABLE_H_

#include "ArrayDefinitions.h"
#include "Permeability.h"

namespace GeoPhysics
{

class PermeabilityImpermeable: public Permeability::Algorithm
{
public:
   PermeabilityImpermeable( double depoPermeability, Permeability::Model model)
      : m_depoPermeability(depoPermeability),
        m_permeabilityValue ( 1.0e-9 )
      , m_model(model)
   {}

   virtual double calculate ( const double ves, const double maxVes, const double calculatedPorosity ) const
   {

      // Added to prevent compiler warnings about unused parameters.
      (void) ves;
      (void) maxVes;
      (void) calculatedPorosity;
      return m_permeabilityValue;
   }

   virtual void calculate ( const unsigned int       n,
                            ArrayDefs::ConstReal_ptr ves,
                            ArrayDefs::ConstReal_ptr maxVes,
                            ArrayDefs::ConstReal_ptr calculatedPorosity,
                            ArrayDefs::Real_ptr      permeabilities ) const
   {

      // Added to prevent compiler warnings about unused parameters.
      (void) ves;
      (void) maxVes;
      (void) calculatedPorosity;

      for ( unsigned int i = 0; i < n; ++i ) {
         permeabilities [ i ] = m_permeabilityValue;
      }

   }

   /// Compte the derivative of the permeability function.
   virtual void calculateDerivative( const double  ves,
                                     const double  maxVes,
                                     const double  calculatedPorosity,
                                     const double  porosityDerivativeWrtVes,
                                           double& permeability,
                                           double& derivative ) const
   {

      // Added to prevent compiler warnings about unused parameters.
      (void) ves;
      (void) maxVes;
      (void) calculatedPorosity;
      (void) porosityDerivativeWrtVes;

      permeability = m_permeabilityValue;
      derivative = 0.0;
   }

   virtual void calculateDerivative ( const unsigned int       n,
                                      ArrayDefs::ConstReal_ptr ves,
                                      ArrayDefs::ConstReal_ptr maxVes,
                                      ArrayDefs::ConstReal_ptr calculatedPorosity,
                                      ArrayDefs::ConstReal_ptr porosityDerivativeWrtVes,
                                      ArrayDefs::Real_ptr      permeabilities,
                                      ArrayDefs::Real_ptr      derivatives ) const
   {


      // Added to prevent compiler warnings about unused parameters.
      (void) ves;
      (void) maxVes;
      (void) calculatedPorosity;
      (void) porosityDerivativeWrtVes;

      for ( unsigned int i = 0; i < n; ++i ) {
         permeabilities [ i ] = m_permeabilityValue;
         derivatives [ i ] = 0.0;
      }

   }

   /// return the depositional permeability
   virtual double depoPerm() const
   {
      return m_depoPermeability;
   }

   /// Return the permeability model
   virtual Permeability::Model  model()    const
   {
      return m_model;
   }

private:
   /// @brief Overwrite default assginment operator
   PermeabilityImpermeable& operator= (const PermeabilityImpermeable&);
   /// @brief Overwrite default copy constructor
   PermeabilityImpermeable( const PermeabilityImpermeable& );

   /// \brief The depositional permeability.
   const double m_depoPermeability;

   /// \brief The permeability value.
   const double m_permeabilityValue;

   /// \brief The permeability model.
   const Permeability::Model m_model;

};



}

#endif
