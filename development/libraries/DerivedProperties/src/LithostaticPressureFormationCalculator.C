//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "DerivedPropertyManager.h"

#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/SimulationDetails.h"
#include "Interface/RunParameters.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicalConstants.h"
#include "GeoPhysicalFunctions.h"
#include "CompoundLithologyArray.h"

#include "LithostaticPressureFormationCalculator.h"
#include "PropertyRetriever.h"

// utilitites library
#include "ConstantsPhysics.h"
#include "ConstantsMathematics.h"

DerivedProperties::LithostaticPressureFormationCalculator::LithostaticPressureFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle ) : m_projectHandle ( projectHandle ) {
   addPropertyName ( "LithoStaticPressure" );

   addDependentPropertyName ( "Ves" );
   addDependentPropertyName ( "Pressure" );

   addDependentPropertyName( "Depth" );
 
   bool hydrostaticDecompactionMode = ( m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" ) != 0 and
                                        ( m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" )->getSimulatorMode () == "HydrostaticDecompaction" ));

   if( not hydrostaticDecompactionMode ) {
      addDependentPropertyName( "Temperature");
   }
   if( m_projectHandle->isALC() ) {
      addDependentPropertyName( "ALCStepTopBasaltDepth");
      addDependentPropertyName( "ALCStepBasaltThickness");
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
                 
                  lithostaticPressure->set ( i, j, k, ( ves->getA ( i, j, k ) * Utilities::Maths::PaToMegaPa + porePressure->getA ( i, j, k )));
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

   const bool alcMode = m_projectHandle->isALC();

   const DataModel::AbstractProperty* aLithostaticPressureProperty = propertyManager.getProperty ( "LithoStaticPressure" );
   
   bool hydrostaticDecompactionMode = ( m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" ) != 0 and
                                        ( m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" )->getSimulatorMode () == "HydrostaticDecompaction"  ));


   const DataModel::AbstractProperty* aDepthProperty = propertyManager.getProperty ( "Depth" );
   const FormationPropertyPtr depth = propertyManager.getFormationProperty ( aDepthProperty, snapshot, formation );

   FormationPropertyPtr temperature;

   if( !hydrostaticDecompactionMode ) {
      const DataModel::AbstractProperty* aTempProperty = propertyManager.getProperty ( "Temperature" );
      temperature = propertyManager.getFormationProperty ( aTempProperty, snapshot, formation );
   }
   FormationMapPropertyPtr basaltDepth;
   FormationMapPropertyPtr basaltThickness;

   if( alcMode ) {
      const DataModel::AbstractProperty* aBasaltDepthProperty = propertyManager.getProperty ( "ALCStepTopBasaltDepth" );
      const DataModel::AbstractProperty* aBasaltThicknessProperty = propertyManager.getProperty ( "ALCStepBasaltThickness" );

      if( formation->getName() != "Crust" ) {
         const GeoPhysics::Formation *mantleFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );
         const DataModel::AbstractFormation * crustFormation = (mantleFormation->getTopSurface()->getTopFormation() );
       
         basaltDepth = propertyManager.getFormationMapProperty ( aBasaltDepthProperty, snapshot, crustFormation );
         basaltThickness = propertyManager.getFormationMapProperty ( aBasaltThicknessProperty, snapshot, crustFormation );
      } else {
         basaltDepth = propertyManager.getFormationMapProperty ( aBasaltDepthProperty, snapshot, formation );
         basaltThickness = propertyManager.getFormationMapProperty ( aBasaltThicknessProperty, snapshot, formation );
      }
   }

   const GeoPhysics::Formation* currentFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );
   
   derivedProperties.clear ();
   
   if( currentFormation!= 0 and depth != 0 and ( hydrostaticDecompactionMode || temperature != 0 ) and 
       ( not alcMode or ( basaltDepth != 0 and basaltThickness != 0 ))) {

      PropertyRetriever depthRetriever ( depth );
      PropertyRetriever tempRetriever;
      PropertyRetriever basaltDepthRetriever;
      PropertyRetriever basaltThicknessRetriever;

      if( !hydrostaticDecompactionMode ) {
         tempRetriever.reset ( temperature );
      }
      if( alcMode ) {
         basaltDepthRetriever.reset( basaltDepth );
         basaltThicknessRetriever.reset( basaltThickness );
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
            
      // Initialise the top set of nodes for the lithostatic pressure
      if ( formationAbove == 0 ) {
         computeLithostaticPressureAtSeaBottom ( propertyManager, snapshot->getTime (), lithostaticPressure );
      } else {
         copyLithostaticPressureFromLayerAbove ( propertyManager, aLithostaticPressureProperty, snapshot, formationAbove, lithostaticPressure );
      }
      
      const GeoPhysics::CompoundLithologyArray& lithologies = currentFormation->getCompoundLithologyArray ();

      double undefinedValue = depth->getUndefinedValue ();
      double segmentThickness, density, pressure, segmentPressure;
      bool constantDensity = hydrostaticDecompactionMode or not alcMode;

 
      for ( unsigned int i = depth->firstI ( true ); i <= depth->lastI ( true ); ++i ) {
         
         for ( unsigned int j = depth->firstJ ( true ); j <= depth->lastJ ( true ); ++j ) {
            
            if ( m_projectHandle->getNodeIsValid ( i, j )) { 
                
               for ( unsigned int k = depth->lastK (); k > depth->firstK (); --k ) {
                  const GeoPhysics::CompoundLithology* lithology = lithologies ( i, j, snapshot->getTime () );

                  if( alcMode ) {
                     const double topBasaltDepth = basaltDepth->getA( i, j );
                     const double botBasaltDepth = topBasaltDepth + 1 + basaltThickness->getA( i, j );

                     if(  basaltThickness->getA( i, j ) != 0 and ( topBasaltDepth <= depth->getA ( i, j, k -1 ) and botBasaltDepth >= depth->getA ( i, j, k - 1 ))) {
                   
                        density = lithology->getSimpleLithology()->getBasaltDensity ( temperature->getA ( i, j, k - 1 ), 
                                                                                      lithostaticPressure->getA ( i, j, k ));
                     } else {
                        density = lithology->getSimpleLithology()->getDensity ( temperature->getA ( i, j, k - 1 ), lithostaticPressure->getA ( i, j, k ));
                     }
                  } else {   
                     density = ( constantDensity ? lithology->getSimpleLithology()->getDensity() :
                                 lithology->getSimpleLithology()->getDensity ( temperature->getA ( i, j, k - 1 ), lithostaticPressure->getA ( i, j, k )));
                  }
                  segmentThickness = depth->getA ( i, j, k - 1 ) - depth->getA ( i, j, k );

                  segmentPressure = segmentThickness * density * Utilities::Physics::AccelerationDueToGravity * Utilities::Maths::PaToMegaPa;

                  pressure = lithostaticPressure->getA ( i, j, k ) + segmentPressure;
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
            lithostaticPressure->set ( i, j, topNodeIndex, lithostaticPressureAbove->getA ( i, j, 0 ));
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

   bool basementFormation = ( dynamic_cast<const GeoPhysics::Formation*>( formation ) != 0 and 
                              dynamic_cast<const GeoPhysics::Formation*>( formation )->kind () == DataAccess::Interface::BASEMENT_FORMATION );

   if( basementFormation ) {
      return isComputableForBasement( propManager, snapshot, formation );
   }

   const std::vector<std::string>& dependentProperties = getDependentPropertyNames ();

   bool propertyIsComputable = true;

   // Determine if the required properties are computable.
   for ( size_t i = 0; i < dependentProperties.size () and propertyIsComputable; ++i ) {

      if( dependentProperties [ i ] == "Temperature" or dependentProperties [ i ] == "ALCStepTopBasaltDepth" or 
          dependentProperties [ i ] == "ALCStepBasaltThickness" ) {
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

bool DerivedProperties::LithostaticPressureFormationCalculator::isComputableForBasement ( const AbstractPropertyManager&      propManager,
                                                                                          const DataModel::AbstractSnapshot*  snapshot,
                                                                                          const DataModel::AbstractFormation* formation ) const {
   
   const std::vector<std::string>& dependentProperties = getDependentPropertyNames ();

   bool propertyIsComputable = true;

   // Determine if the required properties are computable.
   for ( size_t i = 0; i < dependentProperties.size () and propertyIsComputable; ++i ) {

      if( dependentProperties [ i ] == "Ves" or dependentProperties [ i ] == "Pressure" ) {
         propertyIsComputable = true;
      } else {
         const DataModel::AbstractProperty* property = propManager.getProperty ( dependentProperties [ i ]);
            
         if ( property == 0 ) {
            propertyIsComputable = false;
         } else {
            if(  dependentProperties [ i ] == "ALCStepTopBasaltDepth" or dependentProperties [ i ] == "ALCStepBasaltThickness" ) {
               if( formation->getName() != "Crust" ) {
                  const GeoPhysics::Formation *mantleFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );
                  
                  const DataModel::AbstractFormation * crustFormation = (mantleFormation->getTopSurface()->getTopFormation() );
                  propertyIsComputable = propertyIsComputable and propManager.formationMapPropertyIsComputable ( property, snapshot, crustFormation );
               } else {
                  propertyIsComputable = propertyIsComputable and propManager.formationMapPropertyIsComputable ( property, snapshot, formation );
               }
            } else {
               propertyIsComputable = propertyIsComputable and propManager.formationPropertyIsComputable ( property, snapshot, formation );
            }
         }
      }
   }
   return propertyIsComputable;
}
