#ifndef _FASTGENEX6_PROPERTY_MANAGER_H_
#define _FASTGENEX6_PROPERTY_MANAGER_H_

#include "DerivedPropertyManager.h"

namespace GenexSimulation
{
   class PropertyManager : public DerivedProperties::DerivedPropertyManager {

      public :

      PropertyManager ( GeoPhysics::ProjectHandle* projectHandle ); 
      
      ~PropertyManager() {};

   };
}


#endif

