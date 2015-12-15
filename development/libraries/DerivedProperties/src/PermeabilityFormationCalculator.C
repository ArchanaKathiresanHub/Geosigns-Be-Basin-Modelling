#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "DerivedPropertyManager.h"

#include "Interface/RunParameters.h"
#include "Interface/SimulationDetails.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicalConstants.h"
#include "CompoundLithologyArray.h"

#include "PermeabilityFormationCalculator.h"
#include "PropertyRetriever.h"

DerivedProperties::PermeabilityFormationCalculator::PermeabilityFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle ) : m_projectHandle ( projectHandle ) {

   bool chemicalCompactionRequired = m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" ) != 0 and
                                     m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" )->getSimulatorMode () != "HydrostaticDecompaction" and
                                     m_projectHandle->getRunParameters()->getChemicalCompaction ();


   addPropertyName ( "Permeability" );
   addPropertyName ( "HorizontalPermeability" );

   addDependentPropertyName ( "Ves" );
   addDependentPropertyName ( "MaxVes" );

   if ( chemicalCompactionRequired ) {
      addDependentPropertyName ( "ChemicalCompaction" );
   }

}

void DerivedProperties::PermeabilityFormationCalculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                     const DataModel::AbstractSnapshot*          snapshot,
                                                                     const DataModel::AbstractFormation*         formation,
                                                                           FormationPropertyList&                derivedProperties ) const {


   const DataModel::AbstractProperty* aVesProperty = propertyManager.getProperty ( "Ves" );
   const DataModel::AbstractProperty* aMaxVesProperty = propertyManager.getProperty ( "MaxVes" );
   const DataModel::AbstractProperty* aChemicalCompactionProperty = propertyManager.getProperty ( "ChemicalCompaction" );

   const DataModel::AbstractProperty* aPermeabilityVProperty = propertyManager.getProperty ( "Permeability" );
   const DataModel::AbstractProperty* aPermeabilityHProperty = propertyManager.getProperty ( "HorizontalPermeability" );
   
   const FormationPropertyPtr ves    = propertyManager.getFormationProperty ( aVesProperty, snapshot, formation );
   const FormationPropertyPtr maxVes = propertyManager.getFormationProperty ( aMaxVesProperty, snapshot, formation );

   const GeoPhysics::Formation* geoFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );

   PropertyRetriever vesRetriever ( ves );
   PropertyRetriever maxVesRetriever ( maxVes );
   
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
         
         DerivedFormationPropertyPtr verticalPermeability =
            DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( aPermeabilityVProperty, snapshot, formation, 
                                                                                            propertyManager.getMapGrid (), geoFormation->getMaximumNumberOfElements() + 1 ));
         DerivedFormationPropertyPtr horizontalPermeability =
            DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( aPermeabilityHProperty, snapshot, formation, 
                                                                                            propertyManager.getMapGrid (), geoFormation->getMaximumNumberOfElements() + 1 ));
         double undefinedValue = ves->getUndefinedValue ();
         double chemicalCompactionValue, permNorm, permPlane;
         GeoPhysics::CompoundProperty porosity;

         double currentAge = snapshot->getTime();

         for ( unsigned int i = verticalPermeability->firstI ( true ); i <= verticalPermeability->lastI ( true ); ++i ) {
            
            for ( unsigned int j = verticalPermeability->firstJ ( true ); j <= verticalPermeability->lastJ ( true ); ++j ) {
               
               if ( m_projectHandle->getNodeIsValid ( i, j )) {
                  
                  for ( unsigned int k = verticalPermeability->firstK (); k <= verticalPermeability->lastK (); ++k ) {
                     chemicalCompactionValue = ( chemicalCompactionRequired ? chemicalCompaction->get ( i, j, k ) : 0.0 );

                     (*lithologies)( i, j, currentAge )->getPorosity ( ves->get ( i, j, k ) , maxVes->get ( i, j, k ),
                                                           chemicalCompactionRequired, chemicalCompactionValue, porosity );
                     (*lithologies)( i, j, currentAge )->calcBulkPermeabilityNP ( ves->get ( i, j, k ), maxVes->get ( i, j, k ), porosity, permNorm, permPlane );
                     
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
