//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
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

#include "Surface.h"
#include "Snapshot.h"
#include "SimulationDetails.h"
#include "RunParameters.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicalConstants.h"
#include "GeoPhysicalFunctions.h"
#include "CompoundLithologyArray.h"

#include "LithostaticPressureFormationCalculator.h"
#include "PropertyRetriever.h"

// utilitites library
#include "ConstantsPhysics.h"
#include "ConstantsMathematics.h"

using namespace AbstractDerivedProperties;
using namespace std;

DerivedProperties::LithostaticPressureFormationCalculator::LithostaticPressureFormationCalculator ( const GeoPhysics::ProjectHandle& projectHandle ) : m_projectHandle ( projectHandle ) {
   addPropertyName ( "LithoStaticPressure" );

   addDependentPropertyName ( "Ves" );
   addDependentPropertyName ( "Pressure" );

   addDependentPropertyName( "Depth" );

   const DataAccess::Interface::SimulationDetails* lastFastcauldronRun = m_projectHandle.getDetailsOfLastFastcauldron();

   m_hydrostaticDecompactionMode = ( lastFastcauldronRun != 0 and lastFastcauldronRun->getSimulatorMode () == "HydrostaticDecompaction" );

   if( not m_hydrostaticDecompactionMode ) {
      addDependentPropertyName( "Temperature");
   }
   if( m_projectHandle.isALC() ) {
      addDependentPropertyName( "ALCStepTopBasaltDepth");
      addDependentPropertyName( "ALCStepBasaltThickness");
   }
}

double DerivedProperties::LithostaticPressureFormationCalculator::calculateLithostaticPressure(double ves, double porePressure) const
{
  return ves * Utilities::Maths::PaToMegaPa + porePressure;
}

void DerivedProperties::LithostaticPressureFormationCalculator::calculate (       AbstractPropertyManager&      propertyManager,
                                                                            const DataModel::AbstractSnapshot*  snapshot,
                                                                            const DataModel::AbstractFormation* formation,
                                                                                  FormationPropertyList&        derivedProperties ) const {

   const GeoPhysics::GeoPhysicsFormation* geoFormation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation );

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

            if ( m_projectHandle.getNodeIsValid ( i, j )) {

               for ( unsigned int k = ves->firstK (); k <= ves->lastK (); ++k ) {

                  lithostaticPressure->set ( i, j, k, calculateLithostaticPressure(ves->get(i,j,k), porePressure->get(i,j,k)));
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

void DerivedProperties::LithostaticPressureFormationCalculator::calculateForBasement (       AbstractPropertyManager&      propertyManager,
                                                                                       const DataModel::AbstractSnapshot*  snapshot,
                                                                                       const DataModel::AbstractFormation* formation,
                                                                                             FormationPropertyList&        derivedProperties ) const {

   const bool alcMode = m_projectHandle.isALC();

   const DataModel::AbstractProperty* aLithostaticPressureProperty = propertyManager.getProperty ( "LithoStaticPressure" );
   const DataModel::AbstractProperty* aDepthProperty = propertyManager.getProperty ( "Depth" );
   const FormationPropertyPtr depth = propertyManager.getFormationProperty ( aDepthProperty, snapshot, formation );

   FormationPropertyPtr temperature;

   if( not m_hydrostaticDecompactionMode ) {
      const DataModel::AbstractProperty* aTempProperty = propertyManager.getProperty ( "Temperature" );
      temperature = propertyManager.getFormationProperty ( aTempProperty, snapshot, formation );
   }
   FormationMapPropertyPtr basaltDepth;
   FormationMapPropertyPtr basaltThickness;

   if( alcMode ) {
      const DataModel::AbstractProperty* aBasaltDepthProperty = propertyManager.getProperty ( "ALCStepTopBasaltDepth" );
      const DataModel::AbstractProperty* aBasaltThicknessProperty = propertyManager.getProperty ( "ALCStepBasaltThickness" );

      if( formation->getName() != "Crust" ) {
         const GeoPhysics::GeoPhysicsFormation *mantleFormation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation );
         const DataModel::AbstractFormation * crustFormation = (mantleFormation->getTopSurface()->getTopFormation() );

         basaltDepth = propertyManager.getFormationMapProperty ( aBasaltDepthProperty, snapshot, crustFormation );
         basaltThickness = propertyManager.getFormationMapProperty ( aBasaltThicknessProperty, snapshot, crustFormation );
      } else {
         basaltDepth = propertyManager.getFormationMapProperty ( aBasaltDepthProperty, snapshot, formation );
         basaltThickness = propertyManager.getFormationMapProperty ( aBasaltThicknessProperty, snapshot, formation );
      }
   }

   const GeoPhysics::GeoPhysicsFormation* currentFormation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation );

   derivedProperties.clear ();

   if( currentFormation!= 0 and depth != 0 and ( m_hydrostaticDecompactionMode or temperature != 0 ) and
       ( not alcMode or ( basaltDepth != 0 and basaltThickness != 0 ))) {

      PropertyRetriever depthRetriever ( depth );
      PropertyRetriever tempRetriever;
      PropertyRetriever basaltDepthRetriever;
      PropertyRetriever basaltThicknessRetriever;

      if( not m_hydrostaticDecompactionMode ) {
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

      const GeoPhysics::GeoPhysicsFormation* formationAbove = 0;

      if ( currentFormation->getTopSurface ()->getSnapshot () == 0 ||
           currentFormation->getTopSurface ()->getSnapshot ()->getTime () > snapshot->getTime ()) {
         formationAbove = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( currentFormation->getTopSurface ()->getTopFormation ());
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
      bool constantDensity = m_hydrostaticDecompactionMode or not alcMode;


      for ( unsigned int i = depth->firstI ( true ); i <= depth->lastI ( true ); ++i ) {

         for ( unsigned int j = depth->firstJ ( true ); j <= depth->lastJ ( true ); ++j ) {

            if ( m_projectHandle.getNodeIsValid ( i, j )) {

               for ( unsigned int k = depth->lastK (); k > depth->firstK (); --k ) {
                  const GeoPhysics::CompoundLithology* lithology = lithologies ( i, j, snapshot->getTime () );

                  if( alcMode ) {
                     const double topBasaltDepth = basaltDepth->get( i, j );
                     const double botBasaltDepth = topBasaltDepth + 1 + basaltThickness->get( i, j );

                     if(  basaltThickness->get( i, j ) != 0 and ( topBasaltDepth <= depth->get ( i, j, k -1 ) and botBasaltDepth >= depth->get ( i, j, k - 1 ))) {

                        density = lithology->getSimpleLithology()->getBasaltDensity ( temperature->get ( i, j, k - 1 ),
                                                                                      lithostaticPressure->get ( i, j, k ));
                     } else {
                        density = lithology->getSimpleLithology()->getDensity ( temperature->get ( i, j, k - 1 ), lithostaticPressure->get ( i, j, k ));
                     }
                  } else {
                     density = ( constantDensity ? lithology->getSimpleLithology()->getDensity() :
                                 lithology->getSimpleLithology()->getDensity ( temperature->get ( i, j, k - 1 ), lithostaticPressure->get ( i, j, k )));
                  }
                  segmentThickness = depth->get ( i, j, k - 1 ) - depth->get ( i, j, k );

                  segmentPressure = segmentThickness * density * Utilities::Physics::AccelerationDueToGravity * Utilities::Maths::PaToMegaPa;

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
void DerivedProperties::LithostaticPressureFormationCalculator::copyLithostaticPressureFromLayerAbove (       AbstractPropertyManager&      propertyManager,
                                                                                                        const DataModel::AbstractProperty*  lithostaticPressureProperty,
                                                                                                        const DataModel::AbstractSnapshot*  snapshot,
                                                                                                        const DataModel::AbstractFormation* formationAbove,
                                                                                                              DerivedFormationPropertyPtr&  lithostaticPressure ) const {

   const FormationPropertyPtr lithostaticPressureAbove = propertyManager.getFormationProperty ( lithostaticPressureProperty, snapshot, formationAbove );
   double undefinedValue = lithostaticPressureAbove->getUndefinedValue ();
   unsigned int topNodeIndex = lithostaticPressure->lastK ();

   for ( unsigned int i = lithostaticPressureAbove->firstI ( true ); i <= lithostaticPressureAbove->lastI ( true ); ++i ) {

      for ( unsigned int j = lithostaticPressureAbove->firstJ ( true ); j <= lithostaticPressureAbove->lastJ ( true ); ++j ) {

         if ( m_projectHandle.getNodeIsValid ( i, j )) {
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

         if ( m_projectHandle.getNodeIsValid ( i, j )) {
            GeoPhysics::computeHydrostaticPressure ( 0,
                                                     m_projectHandle.getSeaBottomTemperature ( i, j, snapshotAge ),
                                                     m_projectHandle.getSeaBottomDepth ( i, j, snapshotAge ),
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

   bool basementFormation = ( dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation ) != 0 and
                              dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation )->kind () == DataAccess::Interface::BASEMENT_FORMATION );

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
                  const GeoPhysics::GeoPhysicsFormation *mantleFormation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation );

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

double DerivedProperties::LithostaticPressureFormationCalculator::calculateAtPosition( const GeoPhysics::GeoPhysicsFormation* formation,
                                                                                       const GeoPhysics::CompoundLithology* /*lithology*/,
                                                                                       const std::map<string, double>& dependentProperties ) const
{
  if( formation->kind() == DataAccess::Interface::BASEMENT_FORMATION )
  {
     return DataAccess::Interface::DefaultUndefinedScalarValue; // Not Implemented for 0D calculations
  }

  return DerivedProperties::LithostaticPressureFormationCalculator::calculateLithostaticPressure(dependentProperties.at("Ves"),
                                                                                                 dependentProperties.at("Pressure"));
}
