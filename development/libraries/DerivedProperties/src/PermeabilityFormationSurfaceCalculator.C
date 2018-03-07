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
#include "DerivedFormationSurfaceProperty.h"
#include "DerivedPropertyManager.h"

#include "Interface/RunParameters.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicalConstants.h"
#include "CompoundLithologyArray.h"

#include "PermeabilityFormationSurfaceCalculator.h"
#include "PropertyRetriever.h"

// utilities library
#include "ConstantsMathematics.h"

//#define FORMATION_PROPERTY 1

using namespace AbstractDerivedProperties;

DerivedProperties::PermeabilityFormationSurfaceCalculator::PermeabilityFormationSurfaceCalculator ( const GeoPhysics::ProjectHandle* projectHandle ) : m_projectHandle ( projectHandle ) {
   addPropertyName ( "PermeabilityVec2" );
   addPropertyName ( "PermeabilityHVec2" );
}

void DerivedProperties::PermeabilityFormationSurfaceCalculator::calculate (       AbstractPropertyManager&      propertyManager,
                                                                            const DataModel::AbstractSnapshot*  snapshot,
                                                                            const DataModel::AbstractFormation* formation,
                                                                            const DataModel::AbstractSurface*   surface,
                                                                                  FormationSurfacePropertyList& derivedProperties ) const {


   
   const DataModel::AbstractProperty* aVesProperty = propertyManager.getProperty ( "Ves" );
   const DataModel::AbstractProperty* aMaxVesProperty = propertyManager.getProperty ( "MaxVes" );
   const DataModel::AbstractProperty* aChemicalCompactionProperty = propertyManager.getProperty ( "ChemicalCompaction" );

   const DataModel::AbstractProperty* aPermeabilityVProperty = propertyManager.getProperty ( "PermeabilityVec2" );
   const DataModel::AbstractProperty* aPermeabilityHProperty = propertyManager.getProperty ( "PermeabilityHVec2" );
   
#ifndef  FORMATION_PROPERTY
   const SurfacePropertyPtr ves    = propertyManager.getSurfaceProperty ( aVesProperty, snapshot, surface );
   const SurfacePropertyPtr maxVes = propertyManager.getSurfaceProperty ( aMaxVesProperty, snapshot, surface );
#else
   const FormationPropertyPtr ves    = propertyManager.getFormationProperty ( aVesProperty, snapshot, formation );
   const FormationPropertyPtr maxVes = propertyManager.getFormationProperty ( aMaxVesProperty, snapshot, formation );
#endif

   const GeoPhysics::Formation* geoFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );
   
   PropertyRetriever vesRetriever ( ves );
   PropertyRetriever maxVesRetriever ( maxVes );

   derivedProperties.clear ();
   
   if( ves != 0 and maxVes != 0 and geoFormation != 0 ) {
         
      const FormationSurfacePropertyPtr chemicalCompaction = propertyManager.getFormationSurfaceProperty ( aChemicalCompactionProperty, snapshot, formation, surface );
      bool chemicalCompactionRequired  = false;
      
      chemicalCompactionRequired = geoFormation->hasChemicalCompaction () and m_projectHandle->getRunParameters()->getChemicalCompaction () and ( chemicalCompaction != 0 );

      const GeoPhysics::CompoundLithologyArray * lithologies = &geoFormation->getCompoundLithologyArray ();
      
      if(  lithologies != 0 ) {
         
         DerivedFormationSurfacePropertyPtr verticalPermeability =
            DerivedFormationSurfacePropertyPtr ( new DerivedProperties::DerivedFormationSurfaceProperty ( aPermeabilityVProperty, snapshot, 
                                                                                                          formation, surface, propertyManager.getMapGrid () ));
         DerivedFormationSurfacePropertyPtr horizontalPermeability =
            DerivedFormationSurfacePropertyPtr ( new DerivedProperties::DerivedFormationSurfaceProperty ( aPermeabilityHProperty, snapshot, 
                                                                                                          formation, surface, propertyManager.getMapGrid () ));
         double undefinedValue = ves->getUndefinedValue ();
         double currentTime = snapshot->getTime();

         double chemicalCompactionValue, permNorm, permPlane;
         GeoPhysics::CompoundProperty porosity;

#ifdef FORMATION_PROPERTY
         unsigned int vesK = 0;
         unsigned int maxVesK = 0;

         if( surface->getName() == geoFormation->getTopSurfaceName() ) {
            vesK = ves->lastK();
            maxVesK = maxVes->lastK();
         } 
#endif         
         
         for ( unsigned int i = verticalPermeability->firstI ( true ); i <= verticalPermeability->lastI ( true ); ++i ) {
            
            for ( unsigned int j = verticalPermeability->firstJ ( true ); j <= verticalPermeability->lastJ ( true ); ++j ) {
               
               if ( m_projectHandle->getNodeIsValid ( i, j )) {
                 
                     chemicalCompactionValue = ( chemicalCompactionRequired ? chemicalCompaction->get ( i, j ) : 0.0 );

#ifdef FORMATION_PROPERTY
                     (*lithologies)( i, j, currentTime )->getPorosity ( ves->get ( i, j, vesK ), maxVes->get ( i, j, maxVesK ), chemicalCompactionRequired, chemicalCompactionValue, porosity );
                     (*lithologies)( i, j, currentTime )->calcBulkPermeabilityNP ( ves->get ( i, j, vesK ), maxVes->get ( i, j, maxVesK ), porosity, permNorm, permPlane );
#else
                     (*lithologies)( i, j, currentTime )->getPorosity ( ves->get ( i, j ), maxVes->get ( i, j ), chemicalCompactionRequired, chemicalCompactionValue, porosity );
                     (*lithologies)( i, j, currentTime )->calcBulkPermeabilityNP ( ves->get ( i, j ), maxVes->get ( i, j ), porosity, permNorm, permPlane );
#endif                     
                     
                     verticalPermeability->set ( i, j, permNorm / Utilities::Maths::MilliDarcyToM2 );
                     horizontalPermeability->set ( i, j, permPlane / Utilities::Maths::MilliDarcyToM2 );
                      
               } else {
                  verticalPermeability->set ( i, j, undefinedValue );
                  horizontalPermeability->set ( i, j, undefinedValue );
               }
            }
         }
         derivedProperties.push_back ( verticalPermeability );
         derivedProperties.push_back ( horizontalPermeability );
         
      }
   } 
}
