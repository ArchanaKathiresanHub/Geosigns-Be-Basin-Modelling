#ifndef DERIVED_PROPERTIES__POROSITY_CALCULATOR_H
#define DERIVED_PROPERTIES__POROSITY_CALCULATOR_H

#include "FormationPropertyCalculator.h"

namespace DerivedProperties {

   class PorosityFormationCalculator : public FormationPropertyCalculator {

   public :

      PorosityFormationCalculator ();

      virtual ~PorosityFormationCalculator () {}
 
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationPropertyList&        derivedProperties ) const;

      /// \brief Get a list of the property names that will be calculated by the calculator.
      virtual const std::vector<std::string>& getPropertyNames () const;

   private :

      std::vector<std::string> m_propertyNames;
    };


}

#endif 
