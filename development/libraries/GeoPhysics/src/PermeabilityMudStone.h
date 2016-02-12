//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef _GEOPHYSICS__PERMEABILITYMUDSTONE_H_
#define _GEOPHYSICS__PERMEABILITYMUDSTONE_H_

#include "Permeability.h"

namespace GeoPhysics
{

class PermeabilityMudStone: public Permeability::Algorithm
{
public:
   PermeabilityMudStone( double depoPermeability, double permeabilityIncr, double permeabilityDecr);

   /// @brief Compte the mudstone model permeability
   virtual double calculate( const double ves,
                             const double maxVes,
                             const double calculatedPorosity ) const ;

   /// @brief Compte the derivative of the mudstone model permeability function.
   virtual void calculateDerivative( const double  ves,
                                     const double  maxVes,
                                     const double  calculatedPorosity, 
                                     const double porosityDerivativeWrtVes,
                                           double& permeability, 
                                           double& derivative ) const;

   /// return the depositional permeability
   virtual double depoPerm() const ;

   /// Return the permeability model
   virtual Permeability::Model  model()    const;

private:
   /// @brief Overwrite default assginment operator
   PermeabilityMudStone& operator= (const PermeabilityMudStone&);
   /// @brief Overwrite default copy constructor
   PermeabilityMudStone( const PermeabilityMudStone& );

   /// Compute the permeability for shales.
   double shalePermeability ( const double ves,
                              const double maxVes) const;

   /// Compute the derivative of the permeability function for shales.
   void shalePermeabilityAndDerivative ( const double ves,
                                           const double maxVes,
                                           double & permeability,
                                           double & permeabilityDerivative ) const;

   /// A std::pow function for some special cases.
   static inline double fastPow( double x, double y);

   double            m_depoPermeability;
   double            m_permeabilityIncr;
   double            m_permeabilityDecr;

};



}

#endif
