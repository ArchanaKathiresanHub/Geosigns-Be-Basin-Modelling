//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include "FluidVelocityCalculator.h"

#include <iostream>
using namespace std;

#include "FiniteElementTypes.h"
#include "FiniteElement.h"

#include "GeoPhysicsProjectHandle.h"
#include "GeoPhysicsFormation.h"
#include "GeoPhysicsFluidType.h"
#include "GeoPhysicalConstants.h"
#include "CompoundLithology.h"
#include "ElementFunctions.h"

// utilities library
#include "ConstantsMathematics.h"
using Utilities::Maths::M2ToMilliDarcy;
using Utilities::Maths::YearToSecond;

DataAccess::Mining::FluidVelocityCalculator::FluidVelocityCalculator ( const DomainPropertyCollection*            collection,
                                                                       DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                       const Interface::Snapshot*                 snapshot,
                                                                       const Interface::Property*                 property,
                                                                       const RequiredCalculation                  calculation ) :
   DomainFormationProperty ( collection, propertyManager, snapshot, property ),
   m_calculation ( calculation )
{

   m_depth = 0;
   m_temperature = 0;
   m_porosity = 0;
   m_permeabilityN = 0;
   m_permeabilityH = 0;
   m_overpressure = 0;
   m_hydrostaticPressure = 0;

   m_initialised = false;
}

bool DataAccess::Mining::FluidVelocityCalculator::initialise () {

   if ( not m_initialised ) {
      
      m_depth               = getPropertyCollection ()->getDomainProperty ( "Depth", getPropertyManager ());
      m_permeabilityH       = getPropertyCollection ()->getDomainProperty ( "HorizontalPermeability", getPropertyManager ());
      m_hydrostaticPressure = getPropertyCollection ()->getDomainProperty ( "HydroStaticPressure", getPropertyManager ());
      m_overpressure        = getPropertyCollection ()->getDomainProperty ( "OverPressure", getPropertyManager ());
      m_permeabilityN       = getPropertyCollection ()->getDomainProperty ( "Permeability", getPropertyManager ());
      m_porosity            = getPropertyCollection ()->getDomainProperty ( "Porosity", getPropertyManager ());
      m_pressure            = getPropertyCollection ()->getDomainProperty ( "Pressure", getPropertyManager ());
      m_temperature         = getPropertyCollection ()->getDomainProperty ( "Temperature", getPropertyManager ());

      if ( m_depth != 0 and m_temperature != 0 and m_porosity != 0 and m_permeabilityN != 0 and m_permeabilityH != 0 and m_overpressure != 0 and m_hydrostaticPressure != 0 ) {
         m_initialised = m_permeabilityH ->initialise();
         m_initialised = m_initialised and m_permeabilityN ->initialise();
         m_initialised = m_initialised and m_porosity ->initialise();
      } else {
         m_initialised = false;
      }

   }

   return m_initialised;
}

double DataAccess::Mining::FluidVelocityCalculator::compute ( const ElementPosition& position ) const {

   if ( not m_initialised or position.getFormation () == 0 ) {
      return Interface::DefaultUndefinedMapValue;
   }

   const GeoPhysics::GeoPhysicsFormation* formation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>(position.getFormation ());
   const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>(formation->getFluidType ());

   double calculationResult = 0.;

   double fluidViscosity;
   double permeabilityH;
   double permeabilityN;
   double porosity;
   double pressure;
   double temperature;

   FiniteElementMethod::ThreeVector fluidVelocity;
   FiniteElementMethod::Matrix3x3 fluidMobility;
   FiniteElementMethod::FiniteElement finiteElement;

   FiniteElementMethod::ElementVector depthCoeffs;
   FiniteElementMethod::ElementVector overpressureCoeffs;
   FiniteElementMethod::ElementVector hydrostaticPressureCoeffs;

   FiniteElementMethod::ThreeVector gradOverpressure;
   FiniteElementMethod::ThreeVector gradHydrostaticPressure;

   FiniteElementMethod::ElementGeometryMatrix geometryMatrix;

   const GeoPhysics::ProjectHandle* geophysicsProjectHandle = dynamic_cast<const GeoPhysics::ProjectHandle*>(getProjectHandle ());

   if ( fluid == 0 ) {
      return Interface::DefaultUndefinedMapValue;
   }

   dynamic_cast<const DomainFormationProperty*>(m_depth)->extractCoefficients ( position, depthCoeffs );
   dynamic_cast<const DomainFormationProperty*>(m_overpressure)->extractCoefficients ( position, overpressureCoeffs );
   dynamic_cast<const DomainFormationProperty*>(m_hydrostaticPressure)->extractCoefficients ( position, hydrostaticPressureCoeffs );

   getGeometryMatrix ( position,
                       dynamic_cast<const GeoPhysics::ProjectHandle*>(getProjectHandle ())->getCauldronGridDescription (),
                       depthCoeffs, geometryMatrix );

   finiteElement.setGeometry ( geometryMatrix );
   finiteElement.setQuadraturePoint ( position.getReferencePoint ()( 0 ), position.getReferencePoint ()( 1 ), position.getReferencePoint ()( 2 ));

   permeabilityH = m_permeabilityH->compute ( position );
   if( permeabilityH == Interface::DefaultUndefinedMapValue ) {
      return  Interface::DefaultUndefinedMapValue;
   } else {
      permeabilityH /= M2ToMilliDarcy;
   }
   
   permeabilityN = m_permeabilityN->compute ( position );
   if( permeabilityN ==  Interface::DefaultUndefinedMapValue ) {
      return  Interface::DefaultUndefinedMapValue;
   } else {
      permeabilityN /= M2ToMilliDarcy;
   }
   porosity = m_porosity->compute ( position );  
   if( porosity ==  Interface::DefaultUndefinedMapValue ) {
      return  Interface::DefaultUndefinedMapValue;
   } else {
      porosity *= 0.01;
   }
   
   pressure = m_pressure->compute ( position );
   temperature = m_temperature->compute ( position );
      
   fluidViscosity = fluid->viscosity ( temperature, pressure );

   if( pressure  ==  Interface::DefaultUndefinedMapValue or
      temperature ==  Interface::DefaultUndefinedMapValue or
      fluidViscosity ==  Interface::DefaultUndefinedMapValue ) {

     return  Interface::DefaultUndefinedMapValue;
   }
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
   fluidVelocity *= -1000.0 * YearToSecond / porosity;

   switch ( m_calculation ) {
     case X : calculationResult = fluidVelocity ( 1 ); break;
     case Y : calculationResult = fluidVelocity ( 2 ); break;
     case Z : calculationResult = -fluidVelocity ( 3 ); break;
     case MAGNITUDE : calculationResult = length ( fluidVelocity ); break;
   }

   return calculationResult;
}

DataAccess::Mining::FluidVelocityCalculatorAllocator::FluidVelocityCalculatorAllocator ( const FluidVelocityCalculator::RequiredCalculation calculation ) :
   m_calculation ( calculation ) {
}

DataAccess::Mining::DomainProperty* DataAccess::Mining::FluidVelocityCalculatorAllocator::allocate ( const DomainPropertyCollection*            collection,
                                                                                                     DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                                                     const Interface::Snapshot*                 snapshot,
                                                                                                     const Interface::Property*                 property ) const {
   return new FluidVelocityCalculator ( collection, propertyManager, snapshot, property, m_calculation );
}
