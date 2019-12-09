#ifndef _INTERFACE_FLUIDHEATCAPACITYSAMPLE_H_
#define _INTERFACE_FLUIDHEATCAPACITYSAMPLE_H_

#include "DAObject.h"
#include "Interface.h"
#include "FluidType.h"

#include <string>

namespace DataAccess
{
   namespace Interface
   {

      /// A sample point for the fluid heat-capacity.
      ///
      /// A fluid will return a list of these items, describing a 2-d mapping.
      /// (pressure x temperature)->heat-capacity
      class FluidHeatCapacitySample : public DAObject
      {
      public :

         FluidHeatCapacitySample (ProjectHandle& projectHandle, database::Record * record);

         virtual ~FluidHeatCapacitySample ();


         /// Return constant pointer to the fluid to which the heat capacity is associated.
         virtual const FluidType* getFluid () const;

         /// Get temperature value of sample point.
         virtual double getTemperature () const;

         /// Get pressure value of sample point.
         virtual double getPressure () const;

         /// Get heat-capacity value of sample point.
         virtual double getHeatCapacity () const;

      protected :

         const FluidType* m_fluid;

      };

   }
}
#endif // _INTERFACE_FLUIDHEATCAPACITYSAMPLE_H_
