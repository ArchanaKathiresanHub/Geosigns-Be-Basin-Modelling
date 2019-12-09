#ifndef _INTERFACE_FLUIDDENSITYSAMPLE_H_
#define _INTERFACE_FLUIDDENSITYSAMPLE_H_

#include "DAObject.h"
#include "Interface.h"

#include <string>

namespace DataAccess
{
   namespace Interface
   {

      /// A sample point for the fluid density.
      ///
      /// A fluid will return a list of these items, describing a 2-d mapping.
      /// (pressure x temperature)->density
      class FluidDensitySample : public DAObject
      {
      public :

         FluidDensitySample (ProjectHandle& projectHandle, database::Record * record);

         virtual ~FluidDensitySample ();


         /// Return constant pointer to the fluid to which the heat capacity is associated.
         virtual const FluidType* getFluid () const;

         /// Get temperature value of sample point.
         virtual double getTemperature () const;

         /// Get pressure value of sample point.
         virtual double getPressure () const;

         /// Get density value of sample point.
         virtual double getDensity () const;

      protected :

         const FluidType* m_fluid;

      };

   }
}
#endif // _INTERFACE_FLUIDDENSITYSAMPLE_H_
