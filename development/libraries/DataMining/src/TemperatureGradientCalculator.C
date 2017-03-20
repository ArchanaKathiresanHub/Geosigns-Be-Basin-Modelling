#include "TemperatureGradientCalculator.h"

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

DataAccess::Mining::TemperatureGradientCalculator::TemperatureGradientCalculator ( const DomainPropertyCollection*            collection,
                                                                                   DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                                   const Interface::Snapshot*                 snapshot,
                                                                                   const Interface::Property*                 property ) :
   DomainFormationProperty ( collection, propertyManager, snapshot, property )
{

   m_depth = 0;
   m_temperature = 0;
   m_initialised = false;
}

bool DataAccess::Mining::TemperatureGradientCalculator::initialise () {

   if ( not m_initialised ) {
      m_depth = dynamic_cast<const DomainFormationProperty*>(getPropertyCollection ()->getDomainProperty ( "Depth", getPropertyManager ()));
      m_temperature = dynamic_cast<const DomainFormationProperty*>(getPropertyCollection ()->getDomainProperty ( "Temperature", getPropertyManager ()));

      if ( m_depth != 0 and m_temperature != 0 ) {
         m_initialised = true;
      } else {
         m_initialised = false;
      }

   }

   return m_initialised;
}

double DataAccess::Mining::TemperatureGradientCalculator::compute ( const ElementPosition& position ) const {

   if ( not m_initialised or position.getFormation () == 0 ) {
      return Interface::DefaultUndefinedMapValue;
   }

   double calculationResult;

   FiniteElementMethod::FiniteElement finiteElement;
   FiniteElementMethod::ThreeVector   gradTemperature;

   FiniteElementMethod::ElementVector temperatureCoeffs;
   FiniteElementMethod::ElementVector depthCoeffs;

   FiniteElementMethod::ElementGeometryMatrix geometryMatrix;


   dynamic_cast<const DomainFormationProperty*>(m_depth)->extractCoefficients ( position, depthCoeffs );
   dynamic_cast<const DomainFormationProperty*>(m_temperature)->extractCoefficients ( position, temperatureCoeffs );

   getGeometryMatrix ( position,
                       dynamic_cast<const GeoPhysics::ProjectHandle*>(getProjectHandle ())->getCauldronGridDescription (),
                       depthCoeffs, geometryMatrix );

   finiteElement.setGeometry ( geometryMatrix );
   finiteElement.setQuadraturePoint ( position.getReferencePoint ()( 0 ), position.getReferencePoint ()( 1 ), position.getReferencePoint ()( 2 ));

   gradTemperature = finiteElement.interpolateGrad ( temperatureCoeffs );

   // Convert to C/km and get only the vertical gradient.
   calculationResult = 1000.0 * gradTemperature ( 3 );

   return calculationResult;
}

DataAccess::Mining::DomainProperty* DataAccess::Mining::TemperatureGradientCalculatorAllocator::allocate ( const DomainPropertyCollection*            collection,
                                                                                                           DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                                                           const Interface::Snapshot*                 snapshot,
                                                                                                           const Interface::Property*                 property ) const {
   return new TemperatureGradientCalculator ( collection, propertyManager, snapshot, property );
}


