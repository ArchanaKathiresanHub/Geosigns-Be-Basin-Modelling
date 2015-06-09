#include "GenexPropertyManager.h"

#include "LithostaticPressureFormationCalculator.h"
#include "HydrostaticPressureFormationCalculator.h"
#include "PermeabilityFormationCalculator.h"
#include "PorosityFormationCalculator.h"
#include "ErosionFactorFormationMapCalculator.h"

namespace GenexSimulation {

   PropertyManager::PropertyManager ( GeoPhysics::ProjectHandle* projectHandle ) :
      DerivedProperties::DerivedPropertyManager ( projectHandle ) {
      
      addFormationPropertyCalculator ( DerivedProperties::FormationPropertyCalculatorPtr ( new DerivedProperties::HydrostaticPressureFormationCalculator (projectHandle) ));
      addFormationPropertyCalculator ( DerivedProperties::FormationPropertyCalculatorPtr ( new DerivedProperties::LithostaticPressureFormationCalculator (projectHandle) ));
      addFormationPropertyCalculator ( DerivedProperties::FormationPropertyCalculatorPtr ( new DerivedProperties::PermeabilityFormationCalculator (projectHandle) ));
      addFormationPropertyCalculator ( DerivedProperties::FormationPropertyCalculatorPtr ( new DerivedProperties::PorosityFormationCalculator (projectHandle) ));
      addFormationMapPropertyCalculator ( DerivedProperties::FormationMapPropertyCalculatorPtr ( new DerivedProperties::ErosionFactorFormationMapCalculator () ));
  }  

}
 
