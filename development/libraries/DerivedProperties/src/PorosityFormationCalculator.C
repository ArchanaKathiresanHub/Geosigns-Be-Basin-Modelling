#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "DerivedPropertyManager.h"

#include "GeoPhysicsFormation.h"
#include "CompoundLithologyArray.h"

#include "PorosityFormationCalculator.h"

DerivedProperties::PorosityFormationCalculator::PorosityFormationCalculator () {
   m_propertyNames.push_back ( "Porosity" );
}

const std::vector<std::string>& DerivedProperties::PorosityFormationCalculator::getPropertyNames () const {
   return m_propertyNames;
}

void DerivedProperties::PorosityFormationCalculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                 const DataModel::AbstractSnapshot*  snapshot,
                                                                 const DataModel::AbstractFormation* formation,
                                                                       FormationPropertyList&        derivedProperties ) const {

   const std::string fname = formation->getName();
   
   const DataModel::AbstractProperty* aVesProperty = propertyManager.getProperty ( "Ves" );
   const DataModel::AbstractProperty* aMaxVesProperty = propertyManager.getProperty ( "MaxVes" );
   const DataModel::AbstractProperty* aChemicalCompactionProperty = propertyManager.getProperty ( "ChemicalCompaction" );

   const DataModel::AbstractProperty* aPorosityProperty = propertyManager.getProperty ( "Porosity" );
   
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
                  
         DerivedFormationPropertyPtr porosityProp =
            DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( aPorosityProperty, snapshot, formation, 
                                                                                            propertyManager.getMapGrid (), geoFormation->getMaximumNumberOfElements() + 1 ));
         ves->retrieveData();
         maxVes->retrieveData();

         if( chemicalCompactionRequired ) {
            chemicalCompaction->retrieveData();
         }
         double undefinedValue = ves->getUndefinedValue ();
         
         for ( unsigned int i = porosityProp->firstI ( true ); i <= porosityProp->lastI ( true ); ++i ) {
            
            for ( unsigned int j = porosityProp->firstJ ( true ); j <= porosityProp->lastJ ( true ); ++j ) {
               
               if ( propertyManager.getNodeIsValid ( i , j ) ) { //FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
                  
                  for ( unsigned int k = porosityProp->firstK (); k <= porosityProp->lastK (); ++k ) {
                     double chemicalCompactionValue = ( chemicalCompactionRequired ? chemicalCompaction->get ( i, j, k ) : 0.0 );
                     double value = 100.0 * (*lithologies)( i, j )->porosity ( ves->get ( i, j, k ), maxVes->get ( i, j, k ),
                                                                               chemicalCompactionRequired,
                                                                               chemicalCompactionValue );
                     porosityProp->set ( i, j, k, value );
                  }
               } else {
                  for ( unsigned int k = porosityProp->firstK (); k <= porosityProp->lastK (); ++k ) {
                     porosityProp->set ( i, j, k, undefinedValue );
                  }
               }
            }
         }
         derivedProperties.push_back ( porosityProp );

         ves->restoreData();
         maxVes->restoreData();
         
         if( chemicalCompactionRequired ) {
            chemicalCompaction->restoreData();
         }
      }
   }
}
