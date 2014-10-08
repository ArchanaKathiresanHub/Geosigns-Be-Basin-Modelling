#include "AbstractPropertyManager.h"
#include "DerivedFormationSurfaceProperty.h"
#include "DerivedPropertyManager.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicalConstants.h"
#include "CompoundLithologyArray.h"

#include "PermeabilityFormationSurfaceCalculator.h"

DerivedProperties::PermeabilityFormationSurfaceCalculator::PermeabilityFormationSurfaceCalculator () {
   m_propertyNames.push_back ( "PermeabilityVec2" );
   m_propertyNames.push_back ( "PermeabilityHVec2" );
}

const std::vector<std::string>& DerivedProperties::PermeabilityFormationSurfaceCalculator::getPropertyNames () const {
   return m_propertyNames;
}

void DerivedProperties::PermeabilityFormationSurfaceCalculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                            const DataModel::AbstractSnapshot*  snapshot,
                                                                            const DataModel::AbstractFormation* formation,
                                                                            const DataModel::AbstractSurface*   surface,
                                                                            FormationSurfacePropertyList&       derivedProperties ) const {


   const DataModel::AbstractProperty* aVesProperty = propertyManager.getProperty ( "Ves" );
   const DataModel::AbstractProperty* aMaxVesProperty = propertyManager.getProperty ( "MaxVes" );
   const DataModel::AbstractProperty* aChemicalCompactionProperty = propertyManager.getProperty ( "ChemicalCompaction" );

   const DataModel::AbstractProperty* aPermeabilityVProperty = propertyManager.getProperty ( "PermeabilityVec2" );
   const DataModel::AbstractProperty* aPermeabilityHProperty = propertyManager.getProperty ( "PermeabilityHVec2" );
   
   const SurfacePropertyPtr ves    = propertyManager.getSurfaceProperty ( aVesProperty, snapshot, surface );
   const SurfacePropertyPtr maxVes = propertyManager.getSurfaceProperty ( aMaxVesProperty, snapshot, surface );

   const GeoPhysics::Formation* geoFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );
   
   derivedProperties.clear ();
   
   if( ves != 0 and maxVes != 0 and geoFormation != 0 ) {
         
      DerivedProperties::DerivedPropertyManager * dPropertyManager = dynamic_cast< DerivedProperties::DerivedPropertyManager *>( &propertyManager );
      const FormationSurfacePropertyPtr chemicalCompaction = propertyManager.getFormationSurfaceProperty ( aChemicalCompactionProperty, snapshot, formation, surface );
      bool chemicalCompactionRequired  = false;
      
      if( dPropertyManager != 0 ) {
         chemicalCompactionRequired = geoFormation->hasChemicalCompaction () and dPropertyManager->getRunParameters()->getChemicalCompaction () and
            ( chemicalCompaction != 0 );
      }
      const GeoPhysics::CompoundLithologyArray * lithologies = &geoFormation->getCompoundLithologyArray ();
      
      if(  lithologies != 0 ) {
         
         DerivedFormationSurfacePropertyPtr verticalPermeability =
            DerivedFormationSurfacePropertyPtr ( new DerivedProperties::DerivedFormationSurfaceProperty ( aPermeabilityVProperty, snapshot, 
                                                                                                          formation, surface, propertyManager.getMapGrid () ));
         DerivedFormationSurfacePropertyPtr horizontalPermeability =
            DerivedFormationSurfacePropertyPtr ( new DerivedProperties::DerivedFormationSurfaceProperty ( aPermeabilityHProperty, snapshot, 
                                                                                                          formation, surface, propertyManager.getMapGrid () ));
         double undefinedValue = ves->getUndefinedValue ();
         double chemicalCompactionValue, permNorm, permPlane;
         GeoPhysics::CompoundProperty porosity;
         
         for ( unsigned int i = verticalPermeability->firstI ( true ); i <= verticalPermeability->lastI ( true ); ++i ) {
            
            for ( unsigned int j = verticalPermeability->firstJ ( true ); j <= verticalPermeability->lastJ ( true ); ++j ) {
               
               if ( propertyManager.getNodeIsValid ( i , j ) ) { //FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
                  
                     chemicalCompactionValue = ( chemicalCompactionRequired ? chemicalCompaction->get ( i, j ) : 0.0 );

                     (*lithologies)( i, j )->getPorosity ( ves->get ( i, j ), maxVes->get ( i, j ), chemicalCompactionRequired, chemicalCompactionValue, porosity );
                     (*lithologies)( i, j )->calcBulkPermeabilityNP ( ves->get ( i, j ), maxVes->get ( i, j ), porosity, permNorm, permPlane );
                     
                     verticalPermeability->set ( i, j, permNorm / GeoPhysics::MILLIDARCYTOM2 );
                     horizontalPermeability->set ( i, j, permPlane / GeoPhysics::MILLIDARCYTOM2 );
                     
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
