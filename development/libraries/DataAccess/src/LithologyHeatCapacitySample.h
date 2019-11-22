#ifndef _INTERFACE_LITHOLOGYHEATCAPACITYSAMPLE_H_
#define _INTERFACE_LITHOLOGYHEATCAPACITYSAMPLE_H_

#include "DAObject.h"
#include "Interface.h"

#include <string>

namespace DataAccess
{
   namespace Interface
   {

      /// A sample point for the lithology heat-capacity.
      ///
      /// A lithology will return a list of these items, describing a mapping from temperature to heat-capacity.
      /// temperature -> heat-capacity.
      class LithologyHeatCapacitySample : public DAObject
      {
      public :

         LithologyHeatCapacitySample (ProjectHandle& projectHandle, database::Record * record);

         virtual ~LithologyHeatCapacitySample ();

         /// Return the name of the associated lithology.
         virtual const std::string& getLithologyName () const;

         /// Get temperature value of sample point.
         virtual double getTemperature () const;

         /// Get heat-capacity value of sample point.
         virtual double getHeatCapacity () const;

         /// \brief Return the string representation of the heat-capacity sample.
         virtual std::string image () const;

      protected :

         const LithoType* m_lithotype;

      };

   }
}
#endif // _INTERFACE_LITHOLOGYHEATCAPACITYSAMPLE_H_
