#ifndef _GEOPHYSICS__PERMEABILITYSANDSTONE_H_
#define _GEOPHYSICS__PERMEABILITYSANDSTONE_H_

#include "Permeability.h"

namespace GeoPhysics
{

class PermeabilitySandStone: public Permeability::Algorithm
{
public:
   PermeabilitySandStone( double depoPorosity, double depoPermeability, double permeabilityIncr);

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
   virtual Permeability::Model  model()    const ;
  
private:
   double            m_depoPorosity;
   double            m_depoPermeability;
   double            m_permeabilityIncr;

   static const double s_log10;
   static const double s_maxPerm;
};



}

#endif
