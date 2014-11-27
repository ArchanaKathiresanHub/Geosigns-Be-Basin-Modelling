#ifndef DERIVED_PROPERTIES__PROPERTY_CALCULATOR_H
#define DERIVED_PROPERTIES__PROPERTY_CALCULATOR_H

#include "GeoPhysicsProjectHandle.h"

namespace DerivedProperties {

   class PropertyCalculator {

   public :

      PropertyCalculator ( const GeoPhysics::ProjectHandle* projectHandle );

      virtual ~PropertyCalculator ();

      /// \brief Return whether or not the node is defined.
      bool getNodeIsValid ( const unsigned int i, const unsigned int j ) const;

   protected :

      const GeoPhysics::ProjectHandle* getProjectHandle () const;
      
   private :

      const GeoPhysics::ProjectHandle* m_projectHandle;

   };

}


#endif // DERIVED_PROPERTIES__PROPERTY_CALCULATOR_H
