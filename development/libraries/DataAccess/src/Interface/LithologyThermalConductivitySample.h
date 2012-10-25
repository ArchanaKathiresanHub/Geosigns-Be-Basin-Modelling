#ifndef _INTERFACE_LITHOLOGYTHERMALCONDUCTIVITYSAMPLE_H_
#define _INTERFACE_LITHOLOGYTHERMALCONDUCTIVITYSAMPLE_H_

#include "Interface/DAObject.h"
#include "Interface/Interface.h"

#include <string>

namespace DataAccess
{
   namespace Interface
   {

      /// A sample point for the lithology thermal-conductivity.
      ///
      /// A lithology will return a list of these items, describing a mapping from temperature to thermal-conductivity.
      /// temperature -> thermal-conductivity.
      class LithologyThermalConductivitySample : public DAObject
      {
      public :

         LithologyThermalConductivitySample (ProjectHandle * projectHandle, database::Record * record);

         virtual ~LithologyThermalConductivitySample ();


#if 0
         /// Return constant pointer to the lithology to which the thermal-conductivity is associated.
         virtual const LithoType* getLithoType () const;
#endif

         /// \brief Rethrn the name of the lithology to which the sample is associated.
         const std::string& getLithologyName () const;

         /// Get temperature value of sample point.
         virtual double getTemperature () const;

         /// Get thermal-conductivity value of sample point.
         virtual double getThermalConductivity () const;

         /// \brief Return a string representation of the thermal conductivity sample.
         virtual std::string image () const;

      protected :

         const LithoType* m_lithotype;

      };

   }
}
#endif // _INTERFACE_LITHOLOGYTHERMALCONDUCTIVITYSAMPLE_H_
