//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef ABSTRACTDERIVED_PROPERTIES__PROPERTY_CALCULATOR_H
#define ABSTRACTDERIVED_PROPERTIES__PROPERTY_CALCULATOR_H

#include <string>
#include <vector>

namespace AbstractDerivedProperties {

   /// \brief Base class for all property calculators.
   ///
   /// Provides only the names of the properties that can be calculated by the calculator.
   /// And the properties required to be able to calculate these properties.
   class PropertyCalculator {

   public :

      PropertyCalculator () = default;

      virtual ~PropertyCalculator () = default;

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

#endif // ABSTRACTDERIVED_PROPERTIES__PROPERTY_CALCULATOR_H
