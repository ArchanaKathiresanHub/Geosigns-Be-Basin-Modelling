#ifndef _ARRHENIUSMETHODSVRE_H_
#define _ARRHENIUSMETHODSVRE_H_


#include "VitriniteReflectance.h"

#include <vector>
#include <cmath>

namespace GeoPhysics {

class VreArrheniusMethod: public VitriniteReflectance 
{
public:
   VreArrheniusMethod( int numberOfReactions, double preExponentialFactorA, const double * stoichiometricFactors, const double * activationEnergies);
   virtual ~VreArrheniusMethod() {}

   /// Do to the computation for the next time step. 
   virtual void doTimestep( const InputGrid & previousGrid, const InputGrid & currentGrid ) ;

   /// Get the VRe values
   virtual void getResults( OutputGrid & ) ;

   /// Reset the state of the computation
   virtual void reset();
   
protected:
   virtual double convertFractionToVR(double fConv) const = 0;
      
private:
   void initVectors(int gridSize);

   const int m_numberOfReactions;
   const double m_preExponentialFactorA;
   const std::vector<double> m_stoichiometricFactors;
   const std::vector<double> m_activationEnergies;


   /// \Delta I_{ij}, see eq. (9) from Appendix of Sweeney & Burnham (1990)
   std::vector< double > m_deltaI; 

   /// Fraction of reactant converted
   std::vector< double > m_fractionF; 
};


class SweeneyBurnham : public VreArrheniusMethod
{
public:
   SweeneyBurnham() ;

protected:
   virtual double convertFractionToVR(double fConv) const
   {
      /// VR will be higher than s_initialVRe for all fConv values
      double result = std::exp( std::log( s_initialVRe ) + fConv * 3.7 );
      return ( result >= s_initialVRe ) ? result : s_initialVRe;
   }


private:
   static const double s_initialVRe;
};


class Larter : public VreArrheniusMethod
{
public:
   Larter() ;

protected:
   virtual double convertFractionToVR(double fConv) const
   {
      /// VR will be higher than s_initialVRe for all fConv values
      double result = 1.58 - (1. - fConv) * 1.13;
      return ( result >= s_initialVRe ) ? result : s_initialVRe;
   }

private:
   static const double s_initialVRe;
};

} // end namespace GeoPhysics

#endif /** _ARRHENIUSMETHODSVRE_H_ */
