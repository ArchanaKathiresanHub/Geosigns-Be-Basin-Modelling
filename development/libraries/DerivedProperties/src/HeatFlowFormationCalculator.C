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
#include "DerivedPropertyManager.h"

#include "Interface/RunParameters.h"
#include "Interface/SimulationDetails.h"
#include "Interface/Surface.h"

#include "GeoPhysicsFormation.h"

#include "HeatFlowFormationCalculator.h"
#include "PropertyRetriever.h"

// utilities
#include "ConstantsMathematics.h"

using namespace FiniteElementMethod;

DerivedProperties::HeatFlowFormationCalculator::HeatFlowFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle ) : m_projectHandle ( projectHandle ) {


   addPropertyName( "HeatFlowX" );
   addPropertyName( "HeatFlowY" );
   addPropertyName( "HeatFlowZ" );
   
   bool chemicalCompactionRequired = m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" ) != 0 and
                                     m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" )->getSimulatorMode () != "HydrostaticDecompaction" and
                                     m_projectHandle->getRunParameters()->getChemicalCompaction ();

   if ( chemicalCompactionRequired ) {
      addDependentPropertyName ( "ChemicalCompaction" );
   }

   addDependentPropertyName ( "Depth" );
   addDependentPropertyName ( "Temperature" );
   addDependentPropertyName ( "Pressure" );
   addDependentPropertyName ( "OverPressure" );
   addDependentPropertyName ( "Ves" );
   addDependentPropertyName ( "MaxVes" );
   addDependentPropertyName ( "LithoStaticPressure" );

   if( m_projectHandle->isALC() ) {
      addDependentPropertyName ( "ALCStepTopBasaltDepth" );
      addDependentPropertyName ( "ALCStepBasaltThickness" );
    }
}

void DerivedProperties::HeatFlowFormationCalculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                 const DataModel::AbstractSnapshot*  snapshot,
                                                                 const DataModel::AbstractFormation* formation,
                                                                 FormationPropertyList&        derivedProperties ) const {

   const DataModel::AbstractProperty* aDepthProperty        = propertyManager.getProperty ( "Depth" );
   const DataModel::AbstractProperty* aTemperatureProperty  = propertyManager.getProperty ( "Temperature" );
   const DataModel::AbstractProperty* aPressureProperty     = propertyManager.getProperty ( "Pressure" );
   const DataModel::AbstractProperty* aOverpressureProperty = propertyManager.getProperty ( "OverPressure" );
   const DataModel::AbstractProperty* aVesProperty          = propertyManager.getProperty ( "Ves" );
   const DataModel::AbstractProperty* aMaxVesProperty       = propertyManager.getProperty ( "MaxVes" );
   const DataModel::AbstractProperty* aLithoStaticPressure  = propertyManager.getProperty ( "LithoStaticPressure" );
   const DataModel::AbstractProperty* aChemicalCompactionProperty = propertyManager.getProperty ( "ChemicalCompaction" );

   const DataModel::AbstractProperty* heatFlowXProperty = propertyManager.getProperty ( "HeatFlowX" );
   const DataModel::AbstractProperty* heatFlowYProperty = propertyManager.getProperty ( "HeatFlowY" );
   const DataModel::AbstractProperty* heatFlowZProperty = propertyManager.getProperty ( "HeatFlowZ" );

   const FormationPropertyPtr ves          = propertyManager.getFormationProperty ( aVesProperty, snapshot, formation );
   const FormationPropertyPtr maxVes       = propertyManager.getFormationProperty ( aMaxVesProperty, snapshot, formation );
   const FormationPropertyPtr temperature  = propertyManager.getFormationProperty ( aTemperatureProperty, snapshot, formation );
   const FormationPropertyPtr depth        = propertyManager.getFormationProperty ( aDepthProperty, snapshot, formation );
   const FormationPropertyPtr overpressure = propertyManager.getFormationProperty ( aOverpressureProperty, snapshot, formation );
   const FormationPropertyPtr pressure     = propertyManager.getFormationProperty ( aPressureProperty, snapshot, formation );
 
   const GeoPhysics::Formation* geoFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );
 
   // Basement properties
   const bool basementFormation = ( geoFormation->kind () == DataAccess::Interface::BASEMENT_FORMATION );
 
   FormationPropertyPtr lithoStaticPressure;
   if( basementFormation ) {
      lithoStaticPressure = propertyManager.getFormationProperty ( aLithoStaticPressure, snapshot, formation );
   }

   bool basementFormationAndAlcMode = ( basementFormation and m_projectHandle->isALC ());
   FormationMapPropertyPtr basaltDepth;
   FormationMapPropertyPtr basaltThickness;
    
   if ( basementFormationAndAlcMode ) {
      const DataModel::AbstractProperty* basaltDepthProperty     = propertyManager.getProperty ( "ALCStepTopBasaltDepth" );
      const DataModel::AbstractProperty* basaltThicknessProperty = propertyManager.getProperty ( "ALCStepBasaltThickness" );
      
      if( formation->getName() != "Crust" ) {
         const GeoPhysics::Formation *mantleFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );
         const DataModel::AbstractFormation * crustFormation = ( mantleFormation->getTopSurface()->getTopFormation() );
         
         basaltDepth = propertyManager.getFormationMapProperty ( basaltDepthProperty, snapshot, crustFormation );
         basaltThickness = propertyManager.getFormationMapProperty ( basaltThicknessProperty, snapshot, crustFormation );
      } else {
         basaltDepth = propertyManager.getFormationMapProperty ( basaltDepthProperty, snapshot, formation );
         basaltThickness = propertyManager.getFormationMapProperty ( basaltThicknessProperty, snapshot, formation );
      }   
   }
   
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
  
   bool allProperties = temperature != 0 and depth != 0 and geoFormation != 0;

   if( not basementFormation ) {
      allProperties = allProperties and ves != 0 and maxVes != 0 and pressure != 0 and overpressure != 0;
   } else {
      allProperties = allProperties and lithoStaticPressure != 0;
      if( basementFormationAndAlcMode ) {
         allProperties = allProperties and basaltDepth != 0 and basaltThickness != 0;
      }
   }
   if ( chemicalCompactionRequired ) {
      allProperties = allProperties and chemicalCompaction != 0;
   }
        
   if( allProperties ) {
 
      PropertyRetriever temperatureRetriever ( temperature );
      PropertyRetriever porePressureRetriever ( pressure );
      PropertyRetriever overPressureRetriever ( overpressure );
      PropertyRetriever vesRetriever ( ves );
      PropertyRetriever maxVesRetriever ( maxVes );
      PropertyRetriever depthRetriever ( depth );
     
      PropertyRetriever lithoStaticPressureRetriever; 
      PropertyRetriever basaltDepthRetriever; 
      PropertyRetriever basaltThicknessRetriever; 
      
      if( basementFormation ) {
         lithoStaticPressureRetriever.reset( lithoStaticPressure );

         if ( basementFormationAndAlcMode ) {
            basaltDepthRetriever.reset( basaltDepth );
            basaltThicknessRetriever.reset( basaltThickness );
         }
      }

      PropertyRetriever chemicalCompactionRetriever; 
      if ( chemicalCompactionRequired ) {
         chemicalCompactionRetriever.reset( chemicalCompaction );
      } 
      
      const GeoPhysics::CompoundLithologyArray * lithologies = &geoFormation->getCompoundLithologyArray ();
      const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>(geoFormation->getFluidType ());

      double currentTime = snapshot->getTime();
      const bool includeAdvectiveTerm = m_projectHandle->getRunParameters ()->getConvectiveTerm ();
    
      if(  lithologies != 0 ) {

         DerivedFormationPropertyPtr heatFlowX =
            DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( heatFlowXProperty, snapshot, formation, 
                                                                                            propertyManager.getMapGrid (), 
                                                                                            geoFormation->getMaximumNumberOfElements() + 1 ));
         DerivedFormationPropertyPtr heatFlowY =
            DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( heatFlowYProperty, snapshot, formation, 
                                                                                            propertyManager.getMapGrid (), 
                                                                                            geoFormation->getMaximumNumberOfElements() + 1 ));
         DerivedFormationPropertyPtr heatFlowZ =
            DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( heatFlowZProperty, snapshot, formation, 
                                                                                            propertyManager.getMapGrid (), 
                                                                                            geoFormation->getMaximumNumberOfElements() + 1 ));
         double undefinedValue = depth->getUndefinedValue ();

         ElementList elements;
         setUp2dEltMapping( propertyManager, depth, elements );

         unsigned int elementCount;
         unsigned int i, j;
         const GeoPhysics::CompoundLithology* lithology;
 
         ElementGeometryMatrix geometryMatrix;
         ElementVector porePressureVector;
         ElementVector overpressureVector;
         ElementVector lithoPressureVector;
         ElementVector vesVector;
         ElementVector maxVesVector;
         ElementVector temperatureVector;
         ElementVector chemCompactionVector;
         ThreeVector   heatFlow;
         ThreeVector   heatFlowValid;
        
         const double deltaX  = depth->getGrid()->deltaI ();
         const double deltaY  = depth->getGrid()->deltaJ ();
         const double originX = depth->getGrid()->minI ();
         const double originY = depth->getGrid()->minJ ();

         // position at the end of global x, y arrays (the next node is on the border) 
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

                  for (unsigned int k = heatFlowX->firstK(); k < heatFlowX->lastK (); ++k ) {
                     
                     // Retrieve element data.
                     for ( unsigned int node = 1; node <= 8; ++node ) {
                        int LidxZ = k + (( node - 1 ) < 4 ? 1 : 0);
                        int GidxY = elements [ elementCount ].j [( node - 1 ) % 4 ];
                        int GidxX = elements [ elementCount ].i [( node - 1 ) % 4 ];
                        
                        geometryMatrix ( 1, node ) = originX + (deltaX * GidxX);
                        geometryMatrix ( 2, node ) = originY + (deltaY * GidxY);
                        geometryMatrix ( 3, node ) = depth->getA ( GidxX, GidxY, LidxZ );
                        
                        if( not basementFormation ) {
                           vesVector          ( node ) = ves->getA ( GidxX, GidxY, LidxZ );
                           maxVesVector       ( node ) = maxVes->getA ( GidxX, GidxY, LidxZ );
                           overpressureVector ( node ) = overpressure->getA ( GidxX, GidxY, LidxZ );
                           porePressureVector ( node ) = pressure->getA ( GidxX, GidxY, LidxZ );
                        } else {
                           lithoPressureVector ( node ) = lithoStaticPressure->getA ( GidxX, GidxY, LidxZ );
                        }
                        temperatureVector    ( node ) = temperature->getA ( GidxX, GidxY, LidxZ );
                        chemCompactionVector ( node ) = ( chemicalCompactionRequired ? chemicalCompaction->getA ( GidxX, GidxY, LidxZ ) : 0.0 );
                     }
                     
                     bool isBasalt = false;
                     
                     if ( basementFormationAndAlcMode ) {
                        int GidxY = elements [ elementCount ].j [0];
                        int GidxX = elements [ elementCount ].i [0];
                        int LidxZ = k;
                        
                        if( geoFormation->getName() == "Crust" ) {
                           LidxZ = k + 1;
                        }
                        
                        const double topBasaltDepth = basaltDepth->getA(  GidxX, GidxY );
                        const double botBasaltDepth = topBasaltDepth + basaltThickness->getA( GidxX, GidxY ) + 1;
                        
                        if( basaltThickness->getA( GidxX, GidxY ) != 0 and 
                            ( topBasaltDepth <= depth->getA ( GidxX, GidxY, LidxZ ) and botBasaltDepth >= depth->getA (  GidxX, GidxY, LidxZ ))) {
                           
                           isBasalt = true;
                        }
                     }
                     const bool degenerateElement = isDegenerate( geometryMatrix );

                     // compute one  element
                     if( degenerateElement ) {
                        if( validElementsOnSurface and validElementFound ) {
                           // if the element is not valid then copy the values from the last valid element below
                           
                           heatFlow ( 1 ) = heatFlowValid ( 1 );
                           heatFlow ( 2 ) = heatFlowValid ( 2 );
                           heatFlow ( 3 ) = heatFlowValid ( 3 );
                        } else {
                           degenerateElements += 1;
                           
                           heatFlow ( 1 ) = DataAccess::Interface::DefaultUndefinedMapValue;
                           heatFlow ( 2 ) = DataAccess::Interface::DefaultUndefinedMapValue;
                           heatFlow ( 3 ) = DataAccess::Interface::DefaultUndefinedMapValue;
                        }
                     } else {
                        
                        computeHeatFlow ( basementFormation,
                                          isBasalt,
                                          1.0,
                                          lithology,
                                          fluid,
                                          chemicalCompactionRequired,
                                          includeAdvectiveTerm,
                                          geometryMatrix,
                                          vesVector,
                                          maxVesVector,
                                          overpressureVector,
                                          porePressureVector,
                                          lithoPressureVector,
                                          temperatureVector,
                                          chemCompactionVector,
                                          heatFlow );
                        
                        // save this element at a last valid element
                        if( validElementsOnSurface ) {
                           validElementFound = true;
                           
                           heatFlowValid ( 1 ) = heatFlow ( 1 );
                           heatFlowValid ( 2 ) = heatFlow ( 2 );
                           heatFlowValid ( 3 ) = heatFlow ( 3 );
                        }
                     }
                     
                     heatFlowX->set ( i, j, k, heatFlow ( 1 ));
                     heatFlowY->set ( i, j, k, heatFlow ( 2 ));
                     heatFlowZ->set ( i, j, k, heatFlow ( 3 ));
                     
                     fillBorders( i, j, k, globalXNodes, globalYNodes, heatFlowX, heatFlowY, heatFlowZ, heatFlow );

                     if( validElementsOnSurface and degenerateElements > 0 and not degenerateElement ) {
                        // if degenerateElements > 0 then the current element is the first valid element (count from the bottom) - all
                        // elements below are derenerated.
                        // Fill-in the elements below 

                        for ( unsigned int l = 1; l <= degenerateElements ; ++ l ) {
                           heatFlowX->set ( i, j, k - l, heatFlow ( 1 ));
                           heatFlowY->set ( i, j, k - l, heatFlow ( 2 ));
                           heatFlowZ->set ( i, j, k - l, heatFlow ( 3 ));
                           
                           fillBorders( i, j, k - l, globalXNodes, globalYNodes, heatFlowX, heatFlowY, heatFlowZ, heatFlow );
                        }
                        degenerateElements = 0;
                     }

                     // at the top of the formation
                     if( k == heatFlowX->lastK () - 1 ) {
                        if( not degenerateElement ) {
                           computeHeatFlow ( basementFormation,
                                             isBasalt,
                                             -1.0,
                                             lithology,
                                             fluid,
                                             chemicalCompactionRequired,
                                             includeAdvectiveTerm,
                                             geometryMatrix,
                                             vesVector,
                                             maxVesVector,
                                             overpressureVector,
                                             porePressureVector,
                                             lithoPressureVector,
                                             temperatureVector,
                                             chemCompactionVector,
                                             heatFlow );
                        }
                        // if the element is not valid the heatFlow is already pre-filled
                        heatFlowX->set ( i, j, k + 1, heatFlow ( 1 ));
                        heatFlowY->set ( i, j, k + 1, heatFlow ( 2 ));
                        heatFlowZ->set ( i, j, k + 1, heatFlow ( 3 ));
                        
                        fillBorders( i, j, k + 1, globalXNodes, globalYNodes, heatFlowX, heatFlowY, heatFlowZ, heatFlow );
                     } 
                  } // over k loop                  
               } 
            } 
         } // elements loop
         
         derivedProperties.push_back ( heatFlowX );
         derivedProperties.push_back ( heatFlowY );
         derivedProperties.push_back ( heatFlowZ );
      }
   }
}
void DerivedProperties::HeatFlowFormationCalculator::fillBorders( unsigned int i, unsigned int j, unsigned int k,
                                                                  unsigned int globalXNodes, 
                                                                  unsigned int globalYNodes,
                                                                  DerivedFormationPropertyPtr heatFlowX,
                                                                  DerivedFormationPropertyPtr heatFlowY,
                                                                  DerivedFormationPropertyPtr heatFlowZ,
                                                                  ThreeVector heatFlow ) const {
   
   // Fill other heat flow nodes if current (i,j) position is at end of array
   if ( i == globalXNodes ) {
      heatFlowX->set ( i + 1, j, k, heatFlow ( 1 ));
      heatFlowY->set ( i + 1, j, k, heatFlow ( 2 ));
      heatFlowZ->set ( i + 1, j, k, heatFlow ( 3 ));
   }
   
   if ( j == globalYNodes ) {
      heatFlowX->set ( i, j + 1, k, heatFlow ( 1 ));
      heatFlowY->set ( i, j + 1, k, heatFlow ( 2 ));
      heatFlowZ->set ( i, j + 1, k, heatFlow ( 3 ));
   }
   
   if ( i == globalXNodes and j == globalYNodes ) {
      heatFlowX->set ( i + 1, j + 1, k, heatFlow ( 1 ));
      heatFlowY->set ( i + 1, j + 1, k, heatFlow ( 2 ));
      heatFlowZ->set ( i + 1, j + 1, k, heatFlow ( 3 ));
   }
}
     
void DerivedProperties::HeatFlowFormationCalculator::computeHeatFlow ( const bool                    isBasementFormation,
                                                                       const bool                    isBasalt,
                                                                       const double                  z,
                                                                       const GeoPhysics::CompoundLithology*  lithology,
                                                                       const GeoPhysics::FluidType*  fluid,
                                                                       const bool                    includeChemicalCompaction,
                                                                       const bool                    includeAdvectionTerm,
                                                                       const ElementGeometryMatrix&  geometryMatrix,
                                                                       const ElementVector&          currentElementVES,
                                                                       const ElementVector&          currentElementMaxVES,
                                                                       const ElementVector&          currentElementPo,
                                                                       const ElementVector&          currentElementPp,
                                                                       const ElementVector&          currentElementLp,
                                                                       const ElementVector&          currentElementTemperature,
                                                                       const ElementVector&          currentElementChemicalCompaction,
                                                                       ThreeVector&                  heatFlow ) const {

   double conductivityNormal;
   double conductivityTangential;
   double temperature;
   double VES;
   double maxVES;
   double porosity;
   double chemicalCompactionTerm;
   double porePressure;
   
   GradElementVector gradBasis2;
   
   Matrix3x3 conductivityTensor;
   Matrix3x3 permeabilityTensor;

   FiniteElement element;
   element.setGeometry( geometryMatrix );
   element.setQuadraturePoint ( 0, 0, z, true, true );

   GeoPhysics::CompoundProperty currentCompoundPorosity;
   
   temperature = FiniteElementMethod::innerProduct ( currentElementTemperature, element.getBasis() );
   chemicalCompactionTerm = FiniteElementMethod::innerProduct ( currentElementChemicalCompaction, element.getBasis() );

   porosity = 0.0;
   if( isBasementFormation ) {
      double lithostaticPressure  = FiniteElementMethod::innerProduct ( currentElementLp, element.getBasis() );
      if( isBasalt ) {
         lithology -> calcBulkThermCondNPBasalt ( temperature, lithostaticPressure, conductivityNormal, conductivityTangential );
      } else {
         lithology -> calcBulkThermCondNPBasement ( fluid, porosity, temperature, lithostaticPressure, conductivityNormal, conductivityTangential );
      }
   } else {    
      VES          = FiniteElementMethod::innerProduct ( currentElementVES, element.getBasis() );
      maxVES       = FiniteElementMethod::innerProduct ( currentElementMaxVES, element.getBasis() );
      porePressure = FiniteElementMethod::innerProduct ( currentElementPp, element.getBasis() );

      lithology->getPorosity ( VES, maxVES, includeChemicalCompaction, chemicalCompactionTerm, currentCompoundPorosity );
      porosity = currentCompoundPorosity.mixedProperty ();
      
      lithology -> calcBulkThermCondNP ( fluid, porosity, temperature, porePressure, conductivityNormal, conductivityTangential );
   }     
 
   element.setTensor ( conductivityNormal, conductivityTangential, conductivityTensor );
   matrixMatrixProduct ( element.getGradBasis(), conductivityTensor, gradBasis2 );
   matrixTransposeVectorProduct ( gradBasis2, currentElementTemperature, heatFlow );
   
   //
   // Fouriers law states:  q = -k \grad T
   //
   scale ( heatFlow, -1.0 );
   
   // only for sediment
   if ( includeAdvectionTerm and fluid != 0 ) {
      
      double fluidDensity;
      double fluidViscosity;
      double heatCapacity;
      double advectionScaling;
      
      ThreeVector referenceGradOverpressure;
      ThreeVector gradOverpressure;
      ThreeVector fluidFlux;
      
      matrixTransposeVectorProduct ( element.getGradBasis(), currentElementPo, referenceGradOverpressure );
      matrixTransposeVectorProduct ( element.getJacobianInverse(), referenceGradOverpressure, gradOverpressure );

      fluidDensity   = fluid->density ( temperature, porePressure );
      heatCapacity   = fluid->heatCapacity ( temperature, porePressure );
      fluidViscosity = fluid->viscosity ( temperature, porePressure );
      
      // compute fluid Flux

      const double MaximumFlux     = 1.0e-9;
      const double MaximumPermeability = 10.0 * Utilities::Maths::MilliDarcyToM2;
      double permeabilityNormal;
      double permeabilityPlane;
  
      lithology->calcBulkPermeabilityNP ( VES, maxVES, currentCompoundPorosity, permeabilityNormal, permeabilityPlane );

      // Limit the permeability to some maximum value.
      if ( permeabilityNormal > MaximumPermeability ) {
         permeabilityNormal = MaximumPermeability;
      }
      
      // Limit the permeability to some maximum value.
      if ( permeabilityPlane > MaximumPermeability ) {
         permeabilityPlane = MaximumPermeability;
      }
      permeabilityNormal = permeabilityNormal / fluidViscosity;
      permeabilityPlane  = permeabilityPlane  / fluidViscosity;
      
      element.setTensor ( permeabilityNormal, permeabilityPlane, permeabilityTensor );
      matrixVectorProduct ( permeabilityTensor, gradOverpressure, fluidFlux );
   
      // Since pressure properties are stored in MPa units, we must convert to Pa to use in calculation.
      fluidFlux ( 1 ) = -fluidFlux ( 1 ) * Utilities::Maths::MegaPaToPa;
      fluidFlux ( 2 ) = -fluidFlux ( 2 ) * Utilities::Maths::MegaPaToPa;
      fluidFlux ( 3 ) = -fluidFlux ( 3 ) * Utilities::Maths::MegaPaToPa;
      
      // Limit the fluid flux to some maximum value, if requested.
      for ( unsigned int i = 1; i <= 3; ++ i ) {
         
         if ( fluidFlux ( i ) > MaximumFlux ) {
            fluidFlux ( i ) = MaximumFlux;
         } else if ( fluidFlux ( i ) < -MaximumFlux ) {
            fluidFlux ( i ) = -MaximumFlux;
         }
         
      }

      advectionScaling = fluidDensity * heatCapacity * temperature;
      
      heatFlow ( 1 ) = heatFlow ( 1 ) + advectionScaling * fluidFlux ( 1 );
      heatFlow ( 2 ) = heatFlow ( 2 ) + advectionScaling * fluidFlux ( 2 );
      heatFlow ( 3 ) = heatFlow ( 3 ) + advectionScaling * fluidFlux ( 3 );
   }
   
   heatFlow ( 3 ) = -heatFlow ( 3 );
   
   ///
   /// Convert from Watts to milli Watts. 
   ///
   scale ( heatFlow, 1000.0 );
}

bool DerivedProperties::HeatFlowFormationCalculator::isComputable ( const DerivedProperties::AbstractPropertyManager& propManager,
                                                                    const DataModel::AbstractSnapshot*  snapshot,
                                                                    const DataModel::AbstractFormation* formation ) const {
   
   bool basementFormation = ( dynamic_cast<const GeoPhysics::Formation*>( formation ) != 0 and 
                              dynamic_cast<const GeoPhysics::Formation*>( formation )->kind () == DataAccess::Interface::BASEMENT_FORMATION );

   if( basementFormation ) {
      return isComputableForBasement ( propManager, snapshot, formation );
   }

   const std::vector<std::string>& dependentProperties = getDependentPropertyNames ();

   bool propertyIsComputable = true;
   
   // Determine if the required properties are computable.
   for ( size_t i = 0; i < dependentProperties.size () and propertyIsComputable; ++i ) {

      if( dependentProperties [ i ] == "ALCStepTopBasaltDepth" or dependentProperties [ i ] == "ALCStepBasaltThickness" 
          or dependentProperties [ i ] == "LithoStaticPressure" ) {
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

bool DerivedProperties::HeatFlowFormationCalculator::isComputableForBasement ( const AbstractPropertyManager&      propManager,
                                                                               const DataModel::AbstractSnapshot*  snapshot,
                                                                               const DataModel::AbstractFormation* formation ) const {
   
   const std::vector<std::string>& dependentProperties = getDependentPropertyNames ();
   
   bool propertyIsComputable = true;
   
   // Determine if the required properties are computable.
   for ( size_t i = 0; i < dependentProperties.size () and propertyIsComputable; ++i ) {

      if( dependentProperties [ i ] == "Pressure" or dependentProperties [ i ] == "OverPressure" or 
          dependentProperties [ i ] == "Ves" or dependentProperties [ i ] == "MaxVes" or dependentProperties [ i ] == "ChemicalCompaction" ) {

         propertyIsComputable = true;
      } else {
         const DataModel::AbstractProperty* property = propManager.getProperty ( dependentProperties [ i ]);
         
         if ( property == 0 ) {
            propertyIsComputable = false;
         } else {
            if( dependentProperties [ i ] == "ALCStepTopBasaltDepth" or dependentProperties [ i ] == "ALCStepBasaltThickness" ) {
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

bool DerivedProperties::HeatFlowFormationCalculator::isDegenerate ( const ElementGeometryMatrix&  geometryMatrix ) const {

   bool is_Degenerate = true;

   for ( unsigned int i = 1; i <= 4;  ++ i ) {
      is_Degenerate = is_Degenerate && ( fabs ( geometryMatrix ( 3, i ) - geometryMatrix ( 3, i + 4 )) < 0.001 );
   }

   return is_Degenerate;
}


