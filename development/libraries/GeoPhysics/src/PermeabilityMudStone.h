#ifndef _GEOPHYSICS__PERMEABILITYMUDSTONE_H_
#define _GEOPHYSICS__PERMEABILITYMUDSTONE_H_

#include "Permeability.h"

namespace GeoPhysics
{

class PermeabilityMudStone: public Permeability::Algorithm
{
public:
   PermeabilityMudStone( double depoPermeability, double permeabilityIncr, double permeabilityDecr);

   virtual double permeability ( const double ves,
                                 const double maxVes,
                                 const double calculatedPorosity ) const ;

   /// Compte the derivative of the permeability function.
   virtual void permeabilityDerivative ( const double  ves,
                                         const double  maxVes,
                                         const double  calculatedPorosity, 
                                               double& permeability, 
                                               double& derivative ) const;

   /// return the depositional permeability
   virtual double depoPerm() const ;

   /// Return the permeability model
   virtual Permeability::Model  model()    const;

private:
   /// Compute the permeability for shales.
   double shalePermeability ( const double ves,
                              const double maxVes) const;

   /// Compute the derivative of the permeability function for shales.
   double shalePermeabilityDerivative ( const double ves,
                                        const double maxVes ) const;

   double            m_depoPermeability;
   double            m_permeabilityIncr;
   double            m_permeabilityDecr;

   // The natural logarithm of 10
   static const double s_log10;

   // Vertical Effective Stress at surface conditions.
   static const double ves0; 
};



}

#endif
