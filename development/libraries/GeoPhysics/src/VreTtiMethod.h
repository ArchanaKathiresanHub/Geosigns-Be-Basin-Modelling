#ifndef _TTIMETHODSVRE_H_
#define _TTIMETHODSVRE_H_


#include "VitriniteReflectance.h" 

#include <vector>

namespace GeoPhysics {

class VreTtiMethod: public VitriniteReflectance
{
public:
   VreTtiMethod( double factorF, double lopatinP, double lopatinQ );
   virtual ~VreTtiMethod() {}

    /// Do to the computation for the next time step. 
   virtual void doTimestep( const InputGrid & previousGrid, const InputGrid & currentGrid ) ;

   /// Get the VRe values
   virtual void getResults( OutputGrid & ) ;

   /// Reset the state of the computation
   virtual void reset();

private:
   void initVectors(int gridSize);

   static const double s_referenceTemperature;
   static const double s_temperatureIncrement;
   static const double s_initialVRe;

   const double m_factorF;
   const double m_lopatinP;
   const double m_lopatinQ;

   std::vector<double> m_tti; 
};

/*                                                                          F factor   P        Q         */
struct LopatinModified : VreTtiMethod { LopatinModified() : VreTtiMethod( 5.1,       0.08210, 0.62500) {} } ;
struct LopatinGoff     : VreTtiMethod { LopatinGoff()     : VreTtiMethod( 2.0,       0.20799, 1.08680) {} } ;
struct LopatinHood     : VreTtiMethod { LopatinHood()     : VreTtiMethod( 2.0,       0.17340, 0.98875) {} } ;
struct LopatinIssler   : VreTtiMethod { LopatinIssler()   : VreTtiMethod( 2.0,       0.16171, 0.98882) {} } ;
struct LopatinRoyden   : VreTtiMethod { LopatinRoyden()   : VreTtiMethod( 2.0,       0.17300, 0.80360) {} } ;
struct LopatinWaples   : VreTtiMethod { LopatinWaples()   : VreTtiMethod( 2.0,       0.24300, 1.01772) {} } ;
struct LopatinDykstra  : VreTtiMethod { LopatinDykstra()  : VreTtiMethod( 2.0,       0.23697, 1.04431) {} } ;

} // end namespace GeoPhysics

#endif /** _TTIMETHODSVRE_H_ */
