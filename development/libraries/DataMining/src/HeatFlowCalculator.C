//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "HeatFlowCalculator.h"

#include <iostream>
using namespace std;

#include "Interface/RunParameters.h"

#include "FiniteElementTypes.h"
#include "FiniteElement.h"

#include "GeoPhysicsProjectHandle.h"
#include "GeoPhysicsFormation.h"
#include "GeoPhysicsFluidType.h"
#include "GeoPhysicalConstants.h"
#include "CompoundLithology.h"
#include "ElementFunctions.h"

DataAccess::Mining::HeatFlowCalculator::HeatFlowCalculator ( const DomainPropertyCollection*            collection,
                                                             DerivedProperties::DerivedPropertyManager& propertyManager,
                                                             const Interface::Snapshot*                 snapshot,
                                                             const Interface::Property*                 property,
                                                             const RequiredCalculation                  calculation ) :
   DomainFormationProperty ( collection, propertyManager, snapshot, property ),
   m_calculation ( calculation )
{

   m_ves = 0;
   m_maxVes = 0;

   m_depth = 0;
   m_temperature = 0;
   m_overpressure = 0;
   m_hydrostaticPressure = 0;

   m_initialised = false;
}

bool DataAccess::Mining::HeatFlowCalculator::initialise () {

   if ( not m_initialised ) {

      m_ves = getPropertyCollection ()->getDomainProperty ( "Ves", getPropertyManager ());
      m_maxVes = getPropertyCollection ()->getDomainProperty ( "MaxVes", getPropertyManager ());

      m_depth = dynamic_cast<const DomainFormationProperty*>(getPropertyCollection ()->getDomainProperty ( "Depth", getPropertyManager ()));
      m_temperature = dynamic_cast<const DomainFormationProperty*>(getPropertyCollection ()->getDomainProperty ( "Temperature", getPropertyManager ()));
      m_overpressure = dynamic_cast<const DomainFormationProperty*>(getPropertyCollection ()->getDomainProperty ( "OverPressure", getPropertyManager ()));
      m_hydrostaticPressure = dynamic_cast<const DomainFormationProperty*>(getPropertyCollection ()->getDomainProperty ( "HydroStaticPressure", getPropertyManager ()));

      if ( m_ves != 0 and m_maxVes != 0 and m_depth != 0 and m_temperature != 0 and m_overpressure != 0 and m_hydrostaticPressure != 0 ) {
         m_initialised = true;
      } else {
         m_initialised = false;
      }

   }

   return m_initialised;
}

double DataAccess::Mining::HeatFlowCalculator::compute ( const ElementPosition& position ) const {

   if ( not m_initialised or position.getFormation () == 0 ) {
      return Interface::DefaultUndefinedMapValue;
   }

   const bool includeAdvection = getProjectHandle ()->getRunParameters ()->getConvectiveTerm ();

   const GeoPhysics::Formation* formation = dynamic_cast<const GeoPhysics::Formation*>(position.getFormation ());
   const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>(formation->getFluidType ());

   double calculationResult;

   double ves;
   double maxVes;
   double temperature;
   double permeabilityN;
   double permeabilityH;
   double fluidViscosity;
   double fluidDensity;
   double heatCapacity;
   double thermalConductivityN;
   double thermalConductivityH;
   double overpressure;
   double hydrostaticPressure;
   double porePressure;

   GeoPhysics::CompoundProperty porosity;

   FiniteElementMethod::ThreeVector   heatFlow;
   FiniteElementMethod::Matrix3x3     conductivityTensor;
   FiniteElementMethod::FiniteElement finiteElement;
   FiniteElementMethod::ThreeVector   gradTemperature;

   FiniteElementMethod::ElementVector temperatureCoeffs;
   FiniteElementMethod::ElementVector depthCoeffs;
   FiniteElementMethod::ElementVector overpressureCoeffs;
   FiniteElementMethod::ElementVector hydrostaticPressureCoeffs;

   FiniteElementMethod::ElementGeometryMatrix geometryMatrix;

   FiniteElementMethod::ThreeVector fluidVelocity;
   FiniteElementMethod::Matrix3x3   fluidMobility;
   FiniteElementMethod::ThreeVector gradOverpressure;
   FiniteElementMethod::ThreeVector gradHydrostaticPressure;

   if ( fluid == 0 ) {
      return Interface::DefaultUndefinedMapValue;
   }

   const GeoPhysics::Formation* geoForm = dynamic_cast<const GeoPhysics::Formation*>( formation );
   const GeoPhysics::CompoundLithology* lithology = geoForm->getCompoundLithology ( position.getI (), position.getJ ());

   dynamic_cast<const DomainFormationProperty*>(m_depth)->extractCoefficients ( position, depthCoeffs );
   dynamic_cast<const DomainFormationProperty*>(m_temperature)->extractCoefficients ( position, temperatureCoeffs );
   dynamic_cast<const DomainFormationProperty*>(m_overpressure)->extractCoefficients ( position, overpressureCoeffs );
   dynamic_cast<const DomainFormationProperty*>(m_hydrostaticPressure)->extractCoefficients ( position, hydrostaticPressureCoeffs );

   getGeometryMatrix ( position,
                       dynamic_cast<const GeoPhysics::ProjectHandle*>(getProjectHandle ())->getCauldronGridDescription (),
                       depthCoeffs, geometryMatrix );

   finiteElement.setGeometry ( geometryMatrix );
   finiteElement.setQuadraturePoint ( position.getReferencePoint ()( 0 ), position.getReferencePoint ()( 1 ), position.getReferencePoint ()( 2 ));

   ves = m_ves->compute ( position );
   maxVes = m_maxVes->compute ( position );
   temperature = finiteElement.interpolate ( temperatureCoeffs );
   overpressure = finiteElement.interpolate ( overpressureCoeffs );
   hydrostaticPressure = finiteElement.interpolate ( hydrostaticPressureCoeffs ); 
   porePressure = overpressure + hydrostaticPressure;
   
   lithology->getPorosity ( ves, maxVes, false, 0.0, porosity );
   lithology->calcBulkThermCondNP ( fluid, porosity.mixedProperty (), temperature, porePressure, thermalConductivityN, thermalConductivityH );
   finiteElement.setTensor ( thermalConductivityN, thermalConductivityH, conductivityTensor );

   gradTemperature = finiteElement.interpolateGrad ( temperatureCoeffs );

   heatFlow ( 1 ) = conductivityTensor ( 1, 1 ) * gradTemperature ( 1 ) + conductivityTensor ( 1, 2 ) * gradTemperature ( 2 ) + conductivityTensor ( 1, 3 ) * gradTemperature ( 3 );
   heatFlow ( 2 ) = conductivityTensor ( 2, 1 ) * gradTemperature ( 1 ) + conductivityTensor ( 2, 2 ) * gradTemperature ( 2 ) + conductivityTensor ( 2, 3 ) * gradTemperature ( 3 );
   heatFlow ( 3 ) = conductivityTensor ( 3, 1 ) * gradTemperature ( 1 ) + conductivityTensor ( 3, 2 ) * gradTemperature ( 2 ) + conductivityTensor ( 3, 3 ) * gradTemperature ( 3 );

   // Fouriers law states q = -k \grad T.
   heatFlow *= -1.0;

#if 0
   if ( includeAdvection and fluid != 0 ) {
      dynamic_cast<const DomainFormationProperty*>(m_overpressure)->extractCoefficients ( position, overpressureCoeffs );
      dynamic_cast<const DomainFormationProperty*>(m_hydrostaticPressure)->extractCoefficients ( position, hydrostaticPressureCoeffs );

      lithology->calcBulkPermeabilityNP ( ves, maxVes, porosity, permeabilityN, permeabilityH );
      fluidViscosity = fluid->viscosity ( temperature );
      fluidDensity = fluid->density ( temperature, porePressure );
      heatCapacity = fluid->heatCapacity ( temperature, porePressure );

      permeabilityN /= GeoPhysics::M2ToMillyDarcy;
      permeabilityH /= GeoPhysics::M2ToMillyDarcy;

      gradOverpressure = finiteElement.interpolateGrad ( overpressureCoeffs );
      gradHydrostaticPressure = finiteElement.interpolateGrad ( hydrostaticPressureCoeffs );

      // Take care of the surface-topography.
      gradOverpressure ( 1 ) += gradHydrostaticPressure ( 1 );
      gradOverpressure ( 2 ) += gradHydrostaticPressure ( 2 );

      // Convert to Pascals from Mega Pascals.
      gradOverpressure *= 1.0e6;

      permeabilityN /= fluidViscosity;
      permeabilityH /= fluidViscosity;

      finiteElement.setTensor ( permeabilityN, permeabilityH, fluidMobility );

      fluidVelocity ( 1 ) = fluidMobility ( 1, 1 ) * gradOverpressure ( 1 ) + fluidMobility ( 1, 2 ) * gradOverpressure ( 2 ) + fluidMobility ( 1, 3 ) * gradOverpressure ( 3 );
      fluidVelocity ( 2 ) = fluidMobility ( 2, 1 ) * gradOverpressure ( 1 ) + fluidMobility ( 2, 2 ) * gradOverpressure ( 2 ) + fluidMobility ( 2, 3 ) * gradOverpressure ( 3 );
      fluidVelocity ( 3 ) = fluidMobility ( 3, 1 ) * gradOverpressure ( 1 ) + fluidMobility ( 3, 2 ) * gradOverpressure ( 2 ) + fluidMobility ( 3, 3 ) * gradOverpressure ( 3 );

      // Convert to mm/y from m/s.
      fluidVelocity *= -1000.0 * GeoPhysics::YearToSecond / porosity;

      advectionScaling = fluidDensity * heatCapacity * temperature;

      heatFlow ( 1 ) += advectionScaling * fluidVelocity ( 1 );
      heatFlow ( 2 ) += advectionScaling * fluidVelocity ( 2 );
      heatFlow ( 3 ) += advectionScaling * fluidVelocity ( 3 );
   }
#endif

   // Convert to mW/m^2.
   heatFlow *= 1000.0;

   switch ( m_calculation ) {
     case X : calculationResult = heatFlow ( 1 ); break;
     case Y : calculationResult = heatFlow ( 2 ); break;
     case Z : calculationResult = -heatFlow ( 3 ); break;
     case MAGNITUDE : calculationResult = length ( heatFlow ); break;
   }

   return calculationResult;
}

DataAccess::Mining::HeatFlowCalculatorAllocator::HeatFlowCalculatorAllocator ( const HeatFlowCalculator::RequiredCalculation calculation ) :
   m_calculation ( calculation ) {
}

DataAccess::Mining::DomainProperty* DataAccess::Mining::HeatFlowCalculatorAllocator::allocate ( const DomainPropertyCollection*            collection,
                                                                                                DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                                                const Interface::Snapshot*                 snapshot,
                                                                                                const Interface::Property*                 property ) const {
   return new HeatFlowCalculator ( collection, propertyManager, snapshot, property, m_calculation );
}