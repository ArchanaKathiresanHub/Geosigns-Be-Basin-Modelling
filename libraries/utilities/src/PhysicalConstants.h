#ifndef _PHYSICAL_CONSTANTS_H_
#define _PHYSICAL_CONSTANTS_H_

namespace PhysicalConstants {

   /// \var IdealGasConstant
   /// Units are J . K^-1 . mol^-1.
   const double IdealGasConstant = 8.314472;

   /// \var BoltzmannConstant
   /// Units are J . K^-1.
   const double BoltzmannConstant = 1.3806503e-23; 

   /// \var AccelerationDueToGravity
   /// Acceleration due to gravity in m/s^2, more accurate value: 9.80665 m /s^2.
   //  Not really a physical constant.
   const double AccelerationDueToGravity = 9.81;


}


#endif // _PHYSICAL_CONSTANTS_H_
