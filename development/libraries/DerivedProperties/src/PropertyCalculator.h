#ifndef DERIVED_PROPERTIES__PROPERTY_CALCULATOR_H
#define DERIVED_PROPERTIES__PROPERTY_CALCULATOR_H

#include <string>
#include <vector>

namespace DerivedProperties {

   /// \brief Base class for all property calculators.
   ///
   /// Provides only the names of the properties that can be calculated by the calculator.
   /// And the properties required to be able to calculate these properties.
   class PropertyCalculator {

   public :

      PropertyCalculator ();

      virtual ~PropertyCalculator ();

      /// \brief Get a list of the property names that will be calculated by the calculator.
      virtual const std::vector<std::string>& getPropertyNames () const;

      /// \brief Get a list of the property names on which this calculator depends.
      virtual const std::vector<std::string>& getDependentPropertyNames () const;

   protected :

      /// \brief Add property name to the set of properties that will be calculated by this calculator.
      void addPropertyName ( const std::string& propertyName );

      /// \brief Add property name to the set of properties on which this calculator depends.
      void addDependentPropertyName ( const std::string& propertyName );
      
   private :

      /// \brief Contains array of property names.
      std::vector<std::string> m_propertyNames;

      /// \brief Contains array of dependent property names.
      std::vector<std::string> m_dependentPropertyNames;

   };

}

#endif // DERIVED_PROPERTIES__PROPERTY_CALCULATOR_H
