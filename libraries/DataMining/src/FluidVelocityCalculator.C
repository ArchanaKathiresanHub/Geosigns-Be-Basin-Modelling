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

DataAccess::Mining::FluidVelocityCalculator::FluidVelocityCalculator ( const DomainPropertyCollection*  collection,
                                                                       const Interface::Snapshot* snapshot,
                                                                       const Interface::Property* property,
                                                                       const RequiredCalculation  calculation ) :
   DomainFormationProperty ( collection, snapshot, property ),
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
      
      m_depth = getPropertyCollection ()->getDomainProperty ( "Depth" );
      m_temperature = getPropertyCollection ()->getDomainProperty ( "Temperature" );
      m_porosity = getPropertyCollection ()->getDomainProperty ( "Porosity" );
      m_permeabilityN = getPropertyCollection ()->getDomainProperty ( "Permeability" );
      m_permeabilityH = getPropertyCollection ()->getDomainProperty ( "HorizontalPermeability" );
      m_overpressure = getPropertyCollection ()->getDomainProperty ( "OverPressure" );
      m_hydrostaticPressure = getPropertyCollection ()->getDomainProperty ( "HydroStaticPressure" );

      if ( m_depth != 0 and m_temperature != 0 and m_porosity != 0 and m_permeabilityN != 0 and m_permeabilityH != 0 and m_overpressure != 0 and m_hydrostaticPressure != 0 ) {
         m_initialised = true;
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

   const GeoPhysics::Formation* formation = dynamic_cast<const GeoPhysics::Formation*>(position.getFormation ());
   const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>(formation->getFluidType ());

   double fluidVelocityMagnitude;
   double calculationResult;

   double temperature;
   double porosity;
   double permeabilityN;
   double permeabilityH;
   double fluidViscosity;

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

   temperature = m_temperature->compute ( position );
   porosity = 0.01 * m_porosity->compute ( position );
   permeabilityN = m_permeabilityN->compute ( position ) / GeoPhysics::M2TOMILLIDARCY;
   permeabilityH = m_permeabilityH->compute ( position ) / GeoPhysics::M2TOMILLIDARCY;

   fluidViscosity = fluid->viscosity ( temperature );

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
   fluidVelocity *= -1000.0 * GeoPhysics::SecondsPerYear / porosity;

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

DataAccess::Mining::DomainProperty* DataAccess::Mining::FluidVelocityCalculatorAllocator::allocate ( const DomainPropertyCollection*  collection,
                                                                                                     const Interface::Snapshot* snapshot,
                                                                                                     const Interface::Property* property ) const {
   return new FluidVelocityCalculator ( collection, snapshot, property, m_calculation );
}
