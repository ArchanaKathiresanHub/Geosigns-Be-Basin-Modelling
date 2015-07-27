#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "DerivedPropertyManager.h"

#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/SimulationDetails.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicalConstants.h"
#include "GeoPhysicalFunctions.h"
#include "CompoundLithologyArray.h"

#include "LithostaticPressureFormationCalculator.h"
#include "PropertyRetriever.h"

DerivedProperties::LithostaticPressureFormationCalculator::LithostaticPressureFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle ) : m_projectHandle ( projectHandle ) {
   addPropertyName ( "LithoStaticPressure" );

   addDependentPropertyName ( "Ves" );
   addDependentPropertyName ( "Pressure" );

   addDependentPropertyName( "Depth" );
   
   bool hydrostaticDecompactionMode = ( m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" ) != 0 and
                                        ( m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" )->getSimulatorMode () == "HydrostaticDecompaction" ));

   if( !hydrostaticDecompactionMode ) {
      addDependentPropertyName( "Temperature");
   }
}

void DerivedProperties::LithostaticPressureFormationCalculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                            const DataModel::AbstractSnapshot*  snapshot,
                                                                            const DataModel::AbstractFormation* formation,
                                                                                  FormationPropertyList&        derivedProperties ) const {

   const GeoPhysics::Formation* geoFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );

   if( geoFormation != 0 and geoFormation->kind() == DataAccess::Interface::BASEMENT_FORMATION ) {
      return calculateForBasement ( propertyManager, snapshot, formation, derivedProperties );
   }

   const DataModel::AbstractProperty* aVesProperty = propertyManager.getProperty ( "Ves" );
   const DataModel::AbstractProperty* aPorePressureProperty = propertyManager.getProperty ( "Pressure" );

   const DataModel::AbstractProperty* aLithostaticPressureProperty = propertyManager.getProperty ( "LithoStaticPressure" );
   
   const FormationPropertyPtr ves          = propertyManager.getFormationProperty ( aVesProperty, snapshot, formation );
   const FormationPropertyPtr porePressure = propertyManager.getFormationProperty ( aPorePressureProperty, snapshot, formation );

   PropertyRetriever vesRetriever ( ves );
   PropertyRetriever ppRetriever ( porePressure );
   
   derivedProperties.clear ();
   
   if( ves != 0 and porePressure != 0 ) {
              
      DerivedFormationPropertyPtr lithostaticPressure = 
         DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( aLithostaticPressureProperty, snapshot, formation, 
                                                                                         propertyManager.getMapGrid (),
                                                                                         ves->lengthK () ));
      double undefinedValue = ves->getUndefinedValue ();
      
      for ( unsigned int i = ves->firstI ( true ); i <= ves->lastI ( true ); ++i ) {
         
         for ( unsigned int j = ves->firstJ ( true ); j <= ves->lastJ ( true ); ++j ) {
            
            if ( m_projectHandle->getNodeIsValid ( i, j )) { 
                
               for ( unsigned int k = ves->firstK (); k <= ves->lastK (); ++k ) {
                 
                  lithostaticPressure->set ( i, j, k, ( ves->get ( i, j, k ) * GeoPhysics::PascalsToMegaPascals + porePressure->get ( i, j, k )));
               }
            } else {
               for ( unsigned int k = ves->firstK (); k <= ves->lastK (); ++k ) {
                   lithostaticPressure->set ( i, j, k, undefinedValue );
               }
            }
         }
      }

      derivedProperties.push_back ( lithostaticPressure );
   }
}
//------------------------------------------------------------//

void DerivedProperties::LithostaticPressureFormationCalculator::calculateForBasement ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                                       const DataModel::AbstractSnapshot*  snapshot,
                                                                                       const DataModel::AbstractFormation* formation,
                                                                                             FormationPropertyList&        derivedProperties ) const {

    const DataModel::AbstractProperty* aLithostaticPressureProperty = propertyManager.getProperty ( "LithoStaticPressure" );
   
   bool hydrostaticDecompactionMode = ( m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" ) != 0 and
                                        ( m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" )->getSimulatorMode () == "HydrostaticDecompaction" ));


   const DataModel::AbstractProperty* aDepthProperty = propertyManager.getProperty ( "Depth" );
   const FormationPropertyPtr depth       = propertyManager.getFormationProperty ( aDepthProperty, snapshot, formation );

   FormationPropertyPtr temperature;

   if( !hydrostaticDecompactionMode ) {
      const DataModel::AbstractProperty* aTempProperty = propertyManager.getProperty ( "Temperature" );
      temperature = propertyManager.getFormationProperty ( aTempProperty, snapshot, formation );
   }

   const GeoPhysics::Formation* currentFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );
   
   derivedProperties.clear ();
   
   if( currentFormation!= 0 && depth != 0 and ( hydrostaticDecompactionMode || temperature != 0 )) {

      PropertyRetriever depthRetriever ( depth );
      PropertyRetriever tempRetriever;

      if( !hydrostaticDecompactionMode ) {
         tempRetriever.reset ( temperature );
      }

      DerivedFormationPropertyPtr lithostaticPressure = 
         DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( aLithostaticPressureProperty, snapshot, formation, 
                                                                                         propertyManager.getMapGrid (),
                                                                                         depth->lengthK () ));

      const GeoPhysics::Formation* formationAbove = 0;

      if ( currentFormation->getTopSurface ()->getSnapshot () == 0 ||
           currentFormation->getTopSurface ()->getSnapshot ()->getTime () > snapshot->getTime ()) {
         formationAbove = dynamic_cast<const GeoPhysics::Formation*>( currentFormation->getTopSurface ()->getTopFormation ());
      }
            
      // Initialise the top set of nodes for the lithostatic pressure.
      if ( formationAbove == 0 ) {
         computeLithostaticPressureAtSeaBottom ( propertyManager, snapshot->getTime (), lithostaticPressure );
      } else {
         copyLithostaticPressureFromLayerAbove ( propertyManager, aLithostaticPressureProperty, snapshot, formationAbove, lithostaticPressure );
      }
      
      const GeoPhysics::CompoundLithologyArray& lithologies = currentFormation->getCompoundLithologyArray ();

      double undefinedValue = depth->getUndefinedValue ();
      double segmentThickness, density, pressure, segmentPressure;
      bool constantDensity = hydrostaticDecompactionMode || not m_projectHandle->isALC();

      for ( unsigned int i = depth->firstI ( true ); i <= depth->lastI ( true ); ++i ) {
         
         for ( unsigned int j = depth->firstJ ( true ); j <= depth->lastJ ( true ); ++j ) {
            
            if ( m_projectHandle->getNodeIsValid ( i, j )) { 
                
               for ( unsigned int k = depth->lastK (); k > depth->firstK (); --k ) {
                  const GeoPhysics::CompoundLithology* lithology = lithologies ( i, j, snapshot->getTime () );

                  density = ( constantDensity ? lithology->getSimpleLithology()->getDensity() :
                              lithology->getSimpleLithology()->getDensity ( temperature->get ( i, j, k - 1 ), lithostaticPressure->get ( i, j, k )));

                  segmentThickness = depth->get ( i, j, k - 1 ) - depth->get ( i, j, k );

                  segmentPressure = segmentThickness * density * GeoPhysics::AccelerationDueToGravity * GeoPhysics::PascalsToMegaPascals;

                  pressure = lithostaticPressure->get ( i, j, k ) + segmentPressure;
                  lithostaticPressure->set ( i, j, k - 1, pressure );

               }
            } else {
               for ( unsigned int k = lithostaticPressure->firstK (); k <= lithostaticPressure->lastK (); ++k ) {
                  lithostaticPressure->set ( i, j, k, undefinedValue );
               }
            }
         }
      }

      derivedProperties.push_back ( lithostaticPressure );
   }
}

void DerivedProperties::LithostaticPressureFormationCalculator::copyLithostaticPressureFromLayerAbove ( AbstractPropertyManager&            propertyManager,
                                                                                                        const DataModel::AbstractProperty*  lithostaticPressureProperty,
                                                                                                        const DataModel::AbstractSnapshot*  snapshot,
                                                                                                        const DataModel::AbstractFormation* formationAbove,
                                                                                                               DerivedFormationPropertyPtr& lithostaticPressure ) const {
   
   const FormationPropertyPtr lithostaticPressureAbove = propertyManager.getFormationProperty ( lithostaticPressureProperty, snapshot, formationAbove );
   double undefinedValue = lithostaticPressureAbove->getUndefinedValue ();
   unsigned int topNodeIndex = lithostaticPressure->lastK ();

   for ( unsigned int i = lithostaticPressureAbove->firstI ( true ); i <= lithostaticPressureAbove->lastI ( true ); ++i ) {

      for ( unsigned int j = lithostaticPressureAbove->firstJ ( true ); j <= lithostaticPressureAbove->lastJ ( true ); ++j ) {

         if ( m_projectHandle->getNodeIsValid ( i, j )) {
            lithostaticPressure->set ( i, j, topNodeIndex, lithostaticPressureAbove->get ( i, j, 0 ));
         } else {
            lithostaticPressure->set ( i, j, topNodeIndex, undefinedValue );
         }

      }

   }

}

void DerivedProperties::LithostaticPressureFormationCalculator::computeLithostaticPressureAtSeaBottom ( const AbstractPropertyManager&     propertyManager,
                                                                                                        const double                       snapshotAge,
                                                                                                        DerivedFormationPropertyPtr& lithostaticPressure ) const {

   (void) propertyManager;
   double pressure;
   unsigned int topNodeIndex = lithostaticPressure->lastK ();

   for ( unsigned int i = lithostaticPressure->firstI ( true ); i <= lithostaticPressure->lastI ( true ); ++i ) {

      for ( unsigned int j = lithostaticPressure->firstJ ( true ); j <= lithostaticPressure->lastJ ( true ); ++j ) {

         if ( m_projectHandle->getNodeIsValid ( i, j )) {
            GeoPhysics::computeHydrostaticPressure ( 0,
                                                     m_projectHandle->getSeaBottomTemperature ( i, j, snapshotAge ),
                                                     m_projectHandle->getSeaBottomDepth ( i, j, snapshotAge ),
                                                     pressure );
            lithostaticPressure->set ( i, j, topNodeIndex, pressure );
         } else {
            lithostaticPressure->set ( i, j, topNodeIndex, lithostaticPressure->getUndefinedValue ());
         }

      }

   }

}
bool DerivedProperties::LithostaticPressureFormationCalculator::isComputable ( const AbstractPropertyManager&      propManager,
                                                                               const DataModel::AbstractSnapshot*  snapshot,
                                                                               const DataModel::AbstractFormation* formation ) const {

   bool basementFormation = ( dynamic_cast<const GeoPhysics::Formation*>( formation ) != 0 and dynamic_cast<const GeoPhysics::Formation*>( formation )->kind () == DataAccess::Interface::BASEMENT_FORMATION );

   const std::vector<std::string>& dependentProperties = getDependentPropertyNames ();

   bool propertyIsComputable = true;

   // Determine if the required properties are computable.
   for ( size_t i = 0; i < dependentProperties.size () and propertyIsComputable; ++i ) {

      if( basementFormation and ( dependentProperties [ i ] == "Ves" or dependentProperties [ i ] == "Pressure" )) {
         propertyIsComputable = true;
      } else {
         const DataModel::AbstractProperty* property = propManager.getProperty ( dependentProperties [ i ]);

         if ( property == 0 ) {
            propertyIsComputable = false;
         } else {
            propertyIsComputable = propertyIsComputable and propManager.formationPropertyIsComputable ( property, snapshot, formation );
         }
         
      }

   }

   return propertyIsComputable;
}
