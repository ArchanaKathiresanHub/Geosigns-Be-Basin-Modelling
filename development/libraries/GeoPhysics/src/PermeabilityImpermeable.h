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

   virtual double permeability ( const double ves, const double maxVes, const double calculatedPorosity ) const
   {
      return 1.0E-9;
   }

   /// Compte the derivative of the permeability function.
   virtual void permeabilityDerivative ( const double  ves,
                                         const double  maxVes,
                                         const double  calculatedPorosity, 
                                         const double  porosityDerivativeWrtVes,
                                               double& permeability, 
                                               double& derivative ) const
   {
      permeability = 1.0E-9;
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
   double m_depoPermeability;
   Permeability::Model m_model;
};



}

#endif
