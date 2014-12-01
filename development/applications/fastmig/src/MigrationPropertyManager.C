#include "MigrationPropertyManager.h"

#include "PorosityFormationCalculator.h"
#include "PermeabilityFormationCalculator.h"
#include "PermeabilityFormationSurfaceCalculator.h"
#include "VesFormationCalculator.h"
#include "VesSurfaceCalculator.h"
#include "HydrostaticPressureSurfaceCalculator.h"
#include "HydrostaticPressureFormationCalculator.h"

#include "ObjectFactory.h"

migration::MigrationPropertyManager::MigrationPropertyManager ( GeoPhysics::ProjectHandle* projectHandle ) :
   DerivedProperties::DerivedPropertyManager ( projectHandle ) {
   
   addFormationPropertyCalculator ( DerivedProperties::FormationPropertyCalculatorPtr ( new DerivedProperties::PorosityFormationCalculator ( projectHandle )));
   addFormationPropertyCalculator ( DerivedProperties::FormationPropertyCalculatorPtr ( new DerivedProperties::VesFormationCalculator ( projectHandle )));
   addFormationPropertyCalculator ( DerivedProperties::FormationPropertyCalculatorPtr ( new DerivedProperties::PermeabilityFormationCalculator ( projectHandle )));
   addFormationPropertyCalculator ( DerivedProperties::FormationPropertyCalculatorPtr ( new DerivedProperties::HydrostaticPressureFormationCalculator ( projectHandle )));

   addSurfacePropertyCalculator ( DerivedProperties::SurfacePropertyCalculatorPtr ( new DerivedProperties::VesSurfaceCalculator ( projectHandle )));
   addSurfacePropertyCalculator ( DerivedProperties::SurfacePropertyCalculatorPtr ( new DerivedProperties::HydrostaticPressureSurfaceCalculator ( projectHandle )));

   addFormationSurfacePropertyCalculator ( DerivedProperties::FormationSurfacePropertyCalculatorPtr ( new DerivedProperties::PermeabilityFormationSurfaceCalculator ( projectHandle )));
}       
