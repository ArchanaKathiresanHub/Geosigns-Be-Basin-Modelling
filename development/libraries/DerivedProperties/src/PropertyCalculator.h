#ifndef DERIVED_PROPERTIES__PROPERTY_CALCULATOR_H
#define DERIVED_PROPERTIES__PROPERTY_CALCULATOR_H

#include "GeoPhysicsProjectHandle.h"

namespace DerivedProperties {

   class PropertyCalculator {

   public :

      PropertyCalculator ();

      virtual ~PropertyCalculator ();

      /// \brief Get a list of the property names that will be calculated by the calculator.
      virtual const std::vector<std::string>& getPropertyNames () const;

   protected :

      /// \brief Add property name to the set of properties that will be calculated by this calculator.
      void addPropertyName ( const std::string& propertyName );
      
   private :

      /// \brief Contains array of property names.
      std::vector<std::string> m_propertyNames;

   };

}

#endif // DERIVED_PROPERTIES__PROPERTY_CALCULATOR_H
