#ifndef _GEOPHYSICS__PERMEABILITYMULTIPOINT_H_
#define _GEOPHYSICS__PERMEABILITYMULTIPOINT_H_

#include "Permeability.h"

#include "PiecewiseInterpolator.h"

namespace GeoPhysics
{

class PermeabilityMultiPoint: public Permeability::Algorithm
{
public:
   PermeabilityMultiPoint( double depoPorosity, const std::vector<double> & porositySamples, const std::vector<double> & permeabilitySamples); 
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
  // prohibit copying, because ibs::PiecewiseInterpolator has an incorrect copy constructor
  PermeabilityMultiPoint( const PermeabilityMultiPoint & );  

  // prohibit assignment, because ibs::PiecewiseInterpolator has an incorrect copy constructor
  PermeabilityMultiPoint & operator=(const PermeabilityMultiPoint & );

  ibs::PiecewiseInterpolator m_porosityPermeabilityInterpolant;
  double m_depoPermeability;

  static const double s_log10;
};



}

#endif
