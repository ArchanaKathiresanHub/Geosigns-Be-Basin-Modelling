//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
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

   /// @brief Compte the multipoint model permeability
   virtual double calculate ( const double ves,
                              const double maxVes,
                              const double calculatedPorosity ) const ;

   /// Compte the derivative of the multipoint permeability model function
   virtual void calculateDerivative( const double  ves,
                                     const double  maxVes,
                                     const double  calculatedPorosity, 
                                     const double  porosityDerivativeWrtVes,
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
  double m_depoPermeability; ///< The depositional permeability
};



}

#endif
