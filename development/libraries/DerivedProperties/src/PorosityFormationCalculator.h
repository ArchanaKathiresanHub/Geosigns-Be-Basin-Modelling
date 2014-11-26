#ifndef DERIVED_PROPERTIES__POROSITY_CALCULATOR_H
#define DERIVED_PROPERTIES__POROSITY_CALCULATOR_H

#include "FormationPropertyCalculator.h"
#include "GeoPhysicsProjectHandle.h"

namespace DerivedProperties {

   class PorosityFormationCalculator : public FormationPropertyCalculator {

   public :

      PorosityFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle );

      virtual ~PorosityFormationCalculator () {}
 
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractFormation* formation,
                                     FormationPropertyList&        derivedProperties ) const;

      /// \brief Get a list of the property names that will be calculated by the calculator.
      virtual const std::vector<std::string>& getPropertyNames () const;

   private :

      /// \brief Contains array of propert names, in this case only porosity.
      std::vector<std::string> m_propertyNames;

      /// \brief The project handle.
      const GeoPhysics::ProjectHandle* m_projectHandle;

    };


}

#endif 
