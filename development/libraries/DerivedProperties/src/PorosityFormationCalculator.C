#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "DerivedPropertyManager.h"

#include "Interface/RunParameters.h"
#include "Interface/SimulationDetails.h"

#include "GeoPhysicsFormation.h"
#include "CompoundLithologyArray.h"

#include "PropertyRetriever.h"
#include "PorosityFormationCalculator.h"


DerivedProperties::PorosityFormationCalculator::PorosityFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle ) : m_projectHandle ( projectHandle ) {
   addPropertyName ( "Porosity" );

   // It could be that a particular formation does not have chemical-compaction 
   // enabled butit is not possible to determine this here.
   bool chemicalCompactionRequired = m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" ) != 0 and
                                     m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" )->getSimulatorMode () != "HydrostaticDecompaction" and
                                     m_projectHandle->getRunParameters()->getChemicalCompaction ();

   addDependentPropertyName ( "Ves" );
   addDependentPropertyName ( "MaxVes" );

   if ( chemicalCompactionRequired ) {
      addDependentPropertyName ( "ChemicalCompaction" );
   }

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
             
      bool chemicalCompactionRequired = m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" ) != 0 and
                                        m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" )->getSimulatorMode () != "HydrostaticDecompaction" and
                                        geoFormation->hasChemicalCompaction () and m_projectHandle->getRunParameters()->getChemicalCompaction ();

      FormationPropertyPtr chemicalCompaction;

      if ( chemicalCompactionRequired ) {
         chemicalCompaction = propertyManager.getFormationProperty ( aChemicalCompactionProperty, snapshot, formation );
         // Just in case the property is not found.
         chemicalCompactionRequired = chemicalCompaction != 0;
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
         double currentTime = snapshot->getTime();
         
         for ( unsigned int i = porosityProp->firstI ( true ); i <= porosityProp->lastI ( true ); ++i ) {
            
            for ( unsigned int j = porosityProp->firstJ ( true ); j <= porosityProp->lastJ ( true ); ++j ) {
               
               if ( m_projectHandle->getNodeIsValid ( i, j )) {
                  
                  for ( unsigned int k = porosityProp->firstK (); k <= porosityProp->lastK (); ++k ) {
                     double chemicalCompactionValue = ( chemicalCompactionRequired ? chemicalCompaction->getA ( i, j, k ) : 0.0 );
                     double value = 100.0 * (*lithologies)( i, j, currentTime )->porosity ( ves->getA ( i, j, k ), maxVes->getA ( i, j, k ),
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
