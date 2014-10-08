#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "DerivedPropertyManager.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicalConstants.h"
#include "CompoundLithologyArray.h"

#include "PermeabilityFormationCalculator.h"

DerivedProperties::PermeabilityFormationCalculator::PermeabilityFormationCalculator () {
   m_propertyNames.push_back ( "PermeabilityVec2" );
   m_propertyNames.push_back ( "PermeabilityHVec2" );
}

const std::vector<std::string>& DerivedProperties::PermeabilityFormationCalculator::getPropertyNames () const {
   return m_propertyNames;
}

void DerivedProperties::PermeabilityFormationCalculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                     const DataModel::AbstractSnapshot*          snapshot,
                                                                     const DataModel::AbstractFormation*         formation,
                                                                           FormationPropertyList&                derivedProperties ) const {


   const DataModel::AbstractProperty* aVesProperty = propertyManager.getProperty ( "Ves" );
   const DataModel::AbstractProperty* aMaxVesProperty = propertyManager.getProperty ( "MaxVes" );
   const DataModel::AbstractProperty* aChemicalCompactionProperty = propertyManager.getProperty ( "ChemicalCompaction" );

   const DataModel::AbstractProperty* aPermeabilityVProperty = propertyManager.getProperty ( "PermeabilityVec2" );
   const DataModel::AbstractProperty* aPermeabilityHProperty = propertyManager.getProperty ( "PermeabilityHVec2" );
   
   const FormationPropertyPtr ves    = propertyManager.getFormationProperty ( aVesProperty, snapshot, formation );
   const FormationPropertyPtr maxVes = propertyManager.getFormationProperty ( aMaxVesProperty, snapshot, formation );

   const GeoPhysics::Formation* geoFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );
   
   derivedProperties.clear ();
   
   if( ves != 0 and maxVes != 0 and geoFormation != 0 ) {
         
      DerivedProperties::DerivedPropertyManager * dPropertyManager = dynamic_cast< DerivedProperties::DerivedPropertyManager *>( &propertyManager );
      const FormationPropertyPtr chemicalCompaction = propertyManager.getFormationProperty ( aChemicalCompactionProperty, snapshot, formation );
      bool chemicalCompactionRequired  = false;
      
      if( dPropertyManager != 0 ) {
         chemicalCompactionRequired = geoFormation->hasChemicalCompaction () and dPropertyManager->getRunParameters()->getChemicalCompaction () and
            ( chemicalCompaction != 0 );
      }
      const GeoPhysics::CompoundLithologyArray * lithologies = &geoFormation->getCompoundLithologyArray ();
      
      if(  lithologies != 0 ) {
         
         DerivedFormationPropertyPtr verticalPermeability =
            DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( aPermeabilityVProperty, snapshot, formation, 
                                                                                            propertyManager.getMapGrid (), geoFormation->getMaximumNumberOfElements() + 1 ));
         DerivedFormationPropertyPtr horizontalPermeability =
            DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( aPermeabilityHProperty, snapshot, formation, 
                                                                                            propertyManager.getMapGrid (), geoFormation->getMaximumNumberOfElements() + 1 ));
         double undefinedValue = ves->getUndefinedValue ();
         double chemicalCompactionValue, permNorm, permPlane;
         GeoPhysics::CompoundProperty porosity;
         
         for ( unsigned int i = verticalPermeability->firstI ( true ); i <= verticalPermeability->lastI ( true ); ++i ) {
            
            for ( unsigned int j = verticalPermeability->firstJ ( true ); j <= verticalPermeability->lastJ ( true ); ++j ) {
               
               if ( propertyManager.getNodeIsValid ( i , j ) ) { //FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
                  
                  for ( unsigned int k = verticalPermeability->firstK (); k <= verticalPermeability->lastK (); ++k ) {
                     chemicalCompactionValue = ( chemicalCompactionRequired ? chemicalCompaction->get ( i, j, k ) : 0.0 );

                     (*lithologies)( i, j )->getPorosity ( ves->get ( i, j, k ) , maxVes->get ( i, j, k ),
                                                           chemicalCompactionRequired, chemicalCompactionValue, porosity );
                     (*lithologies)( i, j )->calcBulkPermeabilityNP ( ves->get ( i, j, k ), maxVes->get ( i, j, k ), porosity, permNorm, permPlane );
                     
                     verticalPermeability->set ( i, j, k, permNorm / GeoPhysics::MILLIDARCYTOM2 );
                     horizontalPermeability->set ( i, j, k, permPlane / GeoPhysics::MILLIDARCYTOM2 );
                     
                  }
               } else {
                  for ( unsigned int k = verticalPermeability->firstK (); k <= verticalPermeability->lastK (); ++k ) {
                     verticalPermeability->set ( i, j, k, undefinedValue );
                     horizontalPermeability->set ( i, j, k, undefinedValue );
                  }
               }
            }
         }
         derivedProperties.push_back ( verticalPermeability );
         derivedProperties.push_back ( horizontalPermeability );
      }
   }
}
