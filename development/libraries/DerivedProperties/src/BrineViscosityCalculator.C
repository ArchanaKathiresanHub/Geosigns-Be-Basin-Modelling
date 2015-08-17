#include "BrineViscosityCalculator.h"

#include "Interface/RunParameters.h"
#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/SimulationDetails.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicsFluidType.h"
#include "GeoPhysicalConstants.h"
#include "GeoPhysicalFunctions.h"

#include "DerivedFormationProperty.h"
#include "PropertyRetriever.h"

DerivedProperties::BrineViscosityCalculator::BrineViscosityCalculator ( const GeoPhysics::ProjectHandle* projectHandle ) :
   m_projectHandle ( projectHandle )
{
   addPropertyName ( "BrineViscosity" );
   addDependentPropertyName ( "Temperature" );
}


void DerivedProperties::BrineViscosityCalculator::calculate ( AbstractPropertyManager&            propertyManager,
                                                              const DataModel::AbstractSnapshot*  snapshot,
                                                              const DataModel::AbstractFormation* formation,
                                                                    FormationPropertyList&        derivedProperties ) const {

   const GeoPhysics::Formation* currentFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );
   const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>(currentFormation->getFluidType ());

   if( fluid == 0 ) {
      return;
   }

   const DataModel::AbstractProperty* brineViscosityProperty = propertyManager.getProperty ( getPropertyNames ()[ 0 ]);
   DerivedFormationPropertyPtr brineViscosity = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( brineViscosityProperty, snapshot, formation, 
                                                                                                                              propertyManager.getMapGrid (),
                                                                                                                              currentFormation->getMaximumNumberOfElements() + 1 ));

   const DataModel::AbstractProperty* temperatureProperty = propertyManager.getProperty ( "Temperature" );
   FormationPropertyPtr temperature = propertyManager.getFormationProperty ( temperatureProperty, snapshot, formation );

   PropertyRetriever temperatureRetriever ( temperature );

   for ( unsigned int i = brineViscosity->firstI ( true ); i <= brineViscosity->lastI ( true ); ++i ) {

      for ( unsigned int j = brineViscosity->firstJ ( true ); j <= brineViscosity->lastJ ( true ); ++j ) {

         for ( unsigned int k = brineViscosity->firstK (); k <= brineViscosity->lastK (); ++k ) {
            brineViscosity->set ( i, j, k, fluid->viscosity ( temperature->get ( i, j, k )));
         }

      }

   }

   derivedProperties.push_back ( brineViscosity );
}
