#ifndef _INTERFACE_FLUIDTHERMALCONDUCTIVITYSAMPLE_H_
#define _INTERFACE_FLUIDTHERMALCONDUCTIVITYSAMPLE_H_

#include "Interface/DAObject.h"
#include "Interface/Interface.h"

#include <string>

namespace DataAccess
{
   namespace Interface
   {

      /// A sample point for the fluid thermal-conductivity.
      ///
      /// A fluid will return a list of these items, describing a mapping from temperature to thermal-conductivity.
      /// temperature -> thermal-conductivity. 
      class FluidThermalConductivitySample : public DAObject
      {
      public :

         FluidThermalConductivitySample (ProjectHandle * projectHandle, database::Record * record);

         virtual ~FluidThermalConductivitySample ();


         /// Return constant pointer to the fluid to which the thermal-conductivity is associated.
         virtual const FluidType* getFluid () const;

         /// Get temperature value of sample point.
         virtual double getTemperature () const;

         /// Get thermal-conductivity value of sample point.
         virtual double getThermalConductivity () const;

      protected :

         const FluidType* m_fluid;

      };

   }
}
#endif // _INTERFACE_FLUIDTHERMALCONDUCTIVITYSAMPLE_H_
