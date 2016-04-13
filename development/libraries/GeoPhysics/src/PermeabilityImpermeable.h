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

#include "Permeability.h"

namespace GeoPhysics
{

class PermeabilityImpermeable: public Permeability::Algorithm
{
public:
   PermeabilityImpermeable( double depoPermeability, Permeability::Model model)
      : m_depoPermeability(depoPermeability)
      , m_model(model)
   {}

   virtual double calculate ( const double ves, const double maxVes, const double calculatedPorosity ) const
   {
      return 1.0E-9;
   }

   /// Compte the derivative of the permeability function.
   virtual void calculateDerivative( const double  ves,
                                         const double  maxVes,
                                         const double  calculatedPorosity, 
                                         const double  porosityDerivativeWrtVes,
                                               double& permeability, 
                                               double& derivative ) const
   {
      permeability = this->calculate(ves, maxVes, calculatedPorosity);
      derivative = 0.0;
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

   double m_depoPermeability;   ///< The depositional permeability
   Permeability::Model m_model; ///< The permeability model
};



}

#endif
