#include "AbstractPropertyManager.h"
#include "DerivedPropertyManager.h"

#include "Interface/RunParameters.h"
#include "Interface/SimulationDetails.h"

#include "GeoPhysicsFormation.h"

#include "FluidVelocityFormationCalculator.h"
#include "PropertyRetriever.h"

using namespace FiniteElementMethod;

DerivedProperties::FluidVelocityFormationCalculator::FluidVelocityFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle ) : m_projectHandle ( projectHandle ) {

   addPropertyName( "FluidVelocityX" );
   addPropertyName( "FluidVelocityY" );
   addPropertyName( "FluidVelocityZ" );
   
   bool chemicalCompactionRequired = m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" ) != 0 and
                                     m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" )->getSimulatorMode () != "HydrostaticDecompaction" and
                                     m_projectHandle->getRunParameters()->getChemicalCompaction ();

   if ( chemicalCompactionRequired ) {
      addDependentPropertyName ( "ChemicalCompaction" );
   }

   addDependentPropertyName ( "Depth" );
   addDependentPropertyName ( "Temperature" );
   addDependentPropertyName ( "Ves" );
   addDependentPropertyName ( "MaxVes" );
   addDependentPropertyName ( "Pressure" );
}

void DerivedProperties::FluidVelocityFormationCalculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                      const DataModel::AbstractSnapshot*  snapshot,
                                                                      const DataModel::AbstractFormation* formation,
                                                                      FormationPropertyList&        derivedProperties ) const {

   const DataModel::AbstractProperty* aDepthProperty        = propertyManager.getProperty ( "Depth" );
   const DataModel::AbstractProperty* aTemperatureProperty  = propertyManager.getProperty ( "Temperature" );
   const DataModel::AbstractProperty* aVesProperty          = propertyManager.getProperty ( "Ves" );
   const DataModel::AbstractProperty* aMaxVesProperty       = propertyManager.getProperty ( "MaxVes" );
   const DataModel::AbstractProperty* aPorePressureProperty = propertyManager.getProperty ( "Pressure" );
   const DataModel::AbstractProperty* aChemicalCompactionProperty = propertyManager.getProperty ( "ChemicalCompaction" );

   const DataModel::AbstractProperty* fluidVelocityXProperty = propertyManager.getProperty ( "FluidVelocityX" );
   const DataModel::AbstractProperty* fluidVelocityYProperty = propertyManager.getProperty ( "FluidVelocityY" );
   const DataModel::AbstractProperty* fluidVelocityZProperty = propertyManager.getProperty ( "FluidVelocityZ" );

   const FormationPropertyPtr ves          = propertyManager.getFormationProperty ( aVesProperty, snapshot, formation );
   const FormationPropertyPtr maxVes       = propertyManager.getFormationProperty ( aMaxVesProperty, snapshot, formation );
   const FormationPropertyPtr temperature  = propertyManager.getFormationProperty ( aTemperatureProperty, snapshot, formation );
   const FormationPropertyPtr depth        = propertyManager.getFormationProperty ( aDepthProperty, snapshot, formation );
   const FormationPropertyPtr porepressure = propertyManager.getFormationProperty ( aPorePressureProperty, snapshot, formation );
 
   const GeoPhysics::Formation* geoFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );
 
   bool chemicalCompactionRequired = m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" ) != 0 and
      m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" )->getSimulatorMode () != "HydrostaticDecompaction" and
      geoFormation->hasChemicalCompaction () and m_projectHandle->getRunParameters()->getChemicalCompaction ();
   
   FormationPropertyPtr chemicalCompaction;   
   if ( chemicalCompactionRequired ) {
      chemicalCompaction = propertyManager.getFormationProperty ( aChemicalCompactionProperty, snapshot, formation );
      // Just in case the property is not found.
      chemicalCompactionRequired = chemicalCompaction != 0;
   }
   
   derivedProperties.clear ();
  
   bool allProperties = temperature != 0 and depth != 0  and ves != 0 and maxVes != 0  and porepressure != 0 and geoFormation != 0;

   if ( chemicalCompactionRequired ) {
      allProperties = allProperties and chemicalCompaction != 0;
   }
        
   if( allProperties ) {
 
      PropertyRetriever temperatureRetriever ( temperature );
      PropertyRetriever vesRetriever ( ves );
      PropertyRetriever maxVesRetriever ( maxVes );
      PropertyRetriever depthRetriever ( depth );
      PropertyRetriever porePressureRetriever ( porepressure );
    
      PropertyRetriever chemicalCompactionRetriever; 
      if ( chemicalCompactionRequired ) {
         chemicalCompactionRetriever.reset( chemicalCompaction );
      } 
      
      const GeoPhysics::CompoundLithologyArray * lithologies = &geoFormation->getCompoundLithologyArray ();
      const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>(geoFormation->getFluidType ());

      double currentTime = snapshot->getTime();

      if(  lithologies != 0 ) {

         DerivedFormationPropertyPtr fluidVelocityX =
            DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( fluidVelocityXProperty, snapshot, formation, 
                                                                                            propertyManager.getMapGrid (), 
                                                                                            geoFormation->getMaximumNumberOfElements() + 1 ));
         DerivedFormationPropertyPtr fluidVelocityY =
            DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( fluidVelocityYProperty, snapshot, formation, 
                                                                                            propertyManager.getMapGrid (), 
                                                                                            geoFormation->getMaximumNumberOfElements() + 1 ));
         DerivedFormationPropertyPtr fluidVelocityZ =
            DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( fluidVelocityZProperty, snapshot, formation, 
                                                                                            propertyManager.getMapGrid (), 
                                                                                            geoFormation->getMaximumNumberOfElements() + 1 ));

         ElementList elements;
         setUp2dEltMapping( propertyManager, depth, elements );

         unsigned int elementCount;
         unsigned int i, j;
         const GeoPhysics::CompoundLithology* lithology;
 
         ElementGeometryMatrix geometryMatrix;
         ElementVector porepressureVector;
         ElementVector vesVector;
         ElementVector maxVesVector;
         ElementVector temperatureVector;
         ElementVector chemCompactionVector;
         ThreeVector   fluidVelocity;
         ThreeVector   fluidVelocityValid;
        
         const double deltaX  = depth->getGrid()->deltaI ();
         const double deltaY  = depth->getGrid()->deltaJ ();
         const double originX = depth->getGrid()->minI ();
         const double originY = depth->getGrid()->minJ ();
         const unsigned int globalXNodes = depth->getGrid()->numIGlobal() - 2;
         const unsigned int globalYNodes = depth->getGrid()->numJGlobal() - 2;

         // set to true to replace non-valid elements values with the nearest valid element
         const bool validElementsOnSurface = true;

         for ( elementCount = 0; elementCount < elements.size(); elementCount++ ) {
            
            if ( elements[elementCount].exists) {
               i = elements [ elementCount ].i [ 0 ];
               j = elements [ elementCount ].j [ 0 ];
               lithology = (*lithologies)( i, j, currentTime );
               
               if ( lithology != 0 ) {
                  int degenerateElements = 0;
                  bool validElementFound = false;
                  
                  for (unsigned int k = fluidVelocityX->firstK(); k < fluidVelocityX->lastK (); ++k ) {
                     // Retrieve element data.
                     for ( unsigned int node = 1; node <= 8; ++node ) {
                        int LidxZ = k + (( node - 1 ) < 4 ? 1 : 0);
                        int GidxY = elements [ elementCount ].j [( node - 1 ) % 4 ];
                        int GidxX = elements [ elementCount ].i [( node - 1 ) % 4 ];
                        
                        geometryMatrix ( 1, node ) = originX + (deltaX * GidxX);
                        geometryMatrix ( 2, node ) = originY + (deltaY * GidxY);
                        geometryMatrix ( 3, node ) = depth->getA ( GidxX, GidxY, LidxZ );
                        
                        vesVector            ( node ) = ves->getA ( GidxX, GidxY, LidxZ );
                        maxVesVector         ( node ) = maxVes->getA ( GidxX, GidxY, LidxZ );
                        porepressureVector   ( node ) = porepressure->getA ( GidxX, GidxY, LidxZ );
                        temperatureVector    ( node ) = temperature->getA ( GidxX, GidxY, LidxZ );
                        chemCompactionVector ( node ) = ( chemicalCompactionRequired ? chemicalCompaction->getA ( GidxX, GidxY, LidxZ ) : 0.0 );
                     }
                     
                     const bool degenerateElement = isDegenerate( geometryMatrix );
                     
                     // compute one  element
                     if( degenerateElement ) {
                        if( validElementsOnSurface and validElementFound ) {
                           // if the element is not valid then copy the values from the last valid element below
                           fluidVelocity ( 1 ) = fluidVelocityValid ( 1 );
                           fluidVelocity ( 2 ) = fluidVelocityValid ( 2 );
                           fluidVelocity ( 3 ) = fluidVelocityValid ( 3 );
                        } else {
                           degenerateElements += 1;
                           
                           fluidVelocity ( 1 ) = DataAccess::Interface::DefaultUndefinedMapValue;
                           fluidVelocity ( 2 ) = DataAccess::Interface::DefaultUndefinedMapValue;
                           fluidVelocity ( 3 ) = DataAccess::Interface::DefaultUndefinedMapValue;
                        }
                     } else {
                        computeFluidVelocity ( lithology,
                                               1.0,
                                               fluid,
                                               chemicalCompactionRequired,
                                               geometryMatrix,
                                               vesVector,
                                               maxVesVector,
                                               temperatureVector,
                                               chemCompactionVector,
                                               porepressureVector,
                                               fluidVelocity );
                             
                        // save this element at a last valid element
                        if( validElementsOnSurface ) {
                           validElementFound = true;
                           
                           fluidVelocityValid ( 1 ) = fluidVelocity ( 1 );
                           fluidVelocityValid ( 2 ) = fluidVelocity ( 2 );
                           fluidVelocityValid ( 3 ) = fluidVelocity ( 3 );
                        }   
                     }    
            
                     fluidVelocityX->set ( i, j, k, fluidVelocity ( 1 ));
                     fluidVelocityY->set ( i, j, k, fluidVelocity ( 2 ));
                     fluidVelocityZ->set ( i, j, k, fluidVelocity ( 3 ));
                     
                     fillBorders( i, j, k, globalXNodes, globalYNodes, fluidVelocityX, fluidVelocityY, fluidVelocityZ, fluidVelocity );
 
                     if( validElementsOnSurface and degenerateElements > 0 and not degenerateElement ) {
                        // if degenerateElements > 0 then the current element is the first valid element (count from the bottom) - all
                        // elements below are derenerated.
                        // Fill-in the elements below 
                        
                        for ( int l = 1; l <= degenerateElements ; ++ l ) {
                           fluidVelocityX->set ( i, j, k - l, fluidVelocity ( 1 ));
                           fluidVelocityY->set ( i, j, k - l, fluidVelocity ( 2 ));
                           fluidVelocityZ->set ( i, j, k - l, fluidVelocity ( 3 ));
                           
                           fillBorders( i, j, k - l, globalXNodes, globalYNodes, fluidVelocityX, fluidVelocityY, fluidVelocityZ, fluidVelocity );
                        }
                        degenerateElements = 0;
                     }
                     // at the top of the formation
                     if( k == fluidVelocityX->lastK () - 1 ) {
                        if( not degenerateElement ) {
                           computeFluidVelocity (  lithology,
                                                   -1.0,
                                                   fluid,
                                                   chemicalCompactionRequired,
                                                   geometryMatrix,
                                                   vesVector,
                                                   maxVesVector,
                                                   temperatureVector,
                                                   chemCompactionVector,
                                                   porepressureVector,
                                                   fluidVelocity );
                        }

                        // if the element is not valid the fluidVelocity is already pre-filled
                        fluidVelocityX->set ( i, j, k + 1, fluidVelocity ( 1 ));
                        fluidVelocityY->set ( i, j, k + 1, fluidVelocity ( 2 ));
                        fluidVelocityZ->set ( i, j, k + 1, fluidVelocity ( 3 ));
                        
                        fillBorders( i, j, k + 1, globalXNodes, globalYNodes, fluidVelocityX, fluidVelocityY, fluidVelocityZ, fluidVelocity );
                     }
                  }
               }
            }
         }
         derivedProperties.push_back ( fluidVelocityX );
         derivedProperties.push_back ( fluidVelocityY );
         derivedProperties.push_back ( fluidVelocityZ );
      }
   }
}

void DerivedProperties::FluidVelocityFormationCalculator::computeFluidVelocity ( const GeoPhysics::CompoundLithology*  lithology,
                                                                                 const double                  z,
                                                                                 const GeoPhysics::FluidType*  fluid,
                                                                                 const bool                    includeChemicalCompaction,
                                                                                 const ElementGeometryMatrix&  geometryMatrix,
                                                                                 const ElementVector&          currentElementVES,
                                                                                 const ElementVector&          currentElementMaxVES,
                                                                                 const ElementVector&          currentElementTemperature,
                                                                                 const ElementVector&          currentElementChemicalCompaction,
                                                                                 const ElementVector&          currentElementPp,
                                                                                 ThreeVector&                  fluidVelocity ) const {

   double temperature;
   double VES;
   double maxVES;
   double porosity;
   double fluidViscosity;
   double chemicalCompactionTerm;
   double porePressure;
   double relativePermeability;
 
   ThreeVector fluidFlux;
   ThreeVector gradPorePressure;
   ThreeVector referenceGradPorePressure;
   
   if ( fluid == 0 or lithology->surfacePorosity () == 0.0 ) {
      fluidVelocity ( 1 ) = 0.0;
      fluidVelocity ( 2 ) = 0.0;
      fluidVelocity ( 3 ) = 0.0;
      return;
   }
   FiniteElement element;
   element.setGeometry( geometryMatrix );
   element.setQuadraturePoint ( 0, 0, z, true, false );

   GeoPhysics::CompoundProperty currentCompoundPorosity;
   
   temperature  = FiniteElementMethod::innerProduct ( currentElementTemperature, element.getBasis() );
   VES          = FiniteElementMethod::innerProduct ( currentElementVES, element.getBasis() );
   maxVES       = FiniteElementMethod::innerProduct ( currentElementMaxVES, element.getBasis() );
   porePressure = FiniteElementMethod::innerProduct ( currentElementPp, element.getBasis() );
   chemicalCompactionTerm = FiniteElementMethod::innerProduct ( currentElementChemicalCompaction, element.getBasis() );
  
   lithology->getPorosity ( VES, maxVES, includeChemicalCompaction, chemicalCompactionTerm, currentCompoundPorosity );
   porosity = currentCompoundPorosity.mixedProperty ();
 
   fluidViscosity = fluid->viscosity ( temperature, porePressure );
   relativePermeability = fluid->relativePermeability ( temperature, porePressure );

   matrixTransposeVectorProduct ( element.getReferenceGradBasis(), currentElementPp, referenceGradPorePressure );
   matrixTransposeVectorProduct ( element.getJacobianInverse(), referenceGradPorePressure, gradPorePressure );

   // compute fluid Flux
   double permeabilityNormal;
   double permeabilityPlane;
   
   /// Not quite the permeability tensor, since it is also divided by the fluid viscosity.
   Matrix3x3 permeabilityTensor;
   
   lithology->calcBulkPermeabilityNP ( VES, maxVES, currentCompoundPorosity, permeabilityNormal, permeabilityPlane );

   permeabilityNormal = relativePermeability * permeabilityNormal / fluidViscosity;
   permeabilityPlane  = relativePermeability * permeabilityPlane  / fluidViscosity;
   
   element.setTensor ( permeabilityNormal, permeabilityPlane, permeabilityTensor );

   const double fluidDensity = fluid->density( temperature, porePressure );

   gradPorePressure ( 3 ) -= fluidDensity * GeoPhysics::AccelerationDueToGravity * GeoPhysics::Pa_To_MPa;

   matrixVectorProduct ( permeabilityTensor, gradPorePressure, fluidFlux );
   
   // Since pressure properties are stored in MPa units, we must convert to Pa to use in calculation.
   fluidFlux ( 1 ) = -fluidFlux ( 1 ) * GeoPhysics::MPa_To_Pa;
   fluidFlux ( 2 ) = -fluidFlux ( 2 ) * GeoPhysics::MPa_To_Pa;
   fluidFlux ( 3 ) = -fluidFlux ( 3 ) * GeoPhysics::MPa_To_Pa;

   // Convert to mm/year.
   fluidVelocity ( 1 ) =  1000.0 * GeoPhysics::SecondsPerYear * fluidFlux ( 1 ) / porosity;
   fluidVelocity ( 2 ) =  1000.0 * GeoPhysics::SecondsPerYear * fluidFlux ( 2 ) / porosity;
   // +ve to represent upwards, so scale by -1
   fluidVelocity ( 3 ) = -1000.0 * GeoPhysics:: SecondsPerYear * fluidFlux ( 3 ) / porosity;
}

void DerivedProperties::FluidVelocityFormationCalculator::fillBorders( unsigned int i, unsigned int j, unsigned int k,
                                                                       unsigned int globalXNodes, 
                                                                       unsigned int globalYNodes,
                                                                       DerivedFormationPropertyPtr fluidVelocityX,
                                                                       DerivedFormationPropertyPtr fluidVelocityY,
                                                                       DerivedFormationPropertyPtr fluidVelocityZ,
                                                                       ThreeVector fluidVelocity ) const {
   
   // Fill other heat flow nodes if current (i,j) position is at end of array
   if ( i == globalXNodes ) {
      fluidVelocityX->set ( i + 1, j, k, fluidVelocity ( 1 ));
      fluidVelocityY->set ( i + 1, j, k, fluidVelocity ( 2 ));
      fluidVelocityZ->set ( i + 1, j, k, fluidVelocity ( 3 ));
   }
   
   if ( j == globalYNodes ) {
      fluidVelocityX->set ( i, j + 1, k, fluidVelocity ( 1 ));
      fluidVelocityY->set ( i, j + 1, k, fluidVelocity ( 2 ));
      fluidVelocityZ->set ( i, j + 1, k, fluidVelocity ( 3 ));
   }
   
   if ( i == globalXNodes and j == globalYNodes ) {
      fluidVelocityX->set ( i + 1, j + 1, k, fluidVelocity ( 1 ));
      fluidVelocityY->set ( i + 1, j + 1, k, fluidVelocity ( 2 ));
      fluidVelocityZ->set ( i + 1, j + 1, k, fluidVelocity ( 3 ));
   }
}
   
bool DerivedProperties::FluidVelocityFormationCalculator::isDegenerate ( const ElementGeometryMatrix&  geometryMatrix ) const {

   bool is_Degenerate = true;

   for ( unsigned int i = 1; i <= 4;  ++ i ) {
      is_Degenerate = is_Degenerate && ( fabs ( geometryMatrix ( 3, i ) - geometryMatrix ( 3, i + 4 )) < 0.001 );
   }

   return is_Degenerate;
}
