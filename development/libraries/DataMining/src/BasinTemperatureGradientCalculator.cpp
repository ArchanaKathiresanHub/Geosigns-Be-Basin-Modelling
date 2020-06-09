#include "BasinTemperatureGradientCalculator.h"

#include <iostream>
using namespace std;

#include "RunParameters.h"

#include "FiniteElementTypes.h"
#include "FiniteElement.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicsFluidType.h"
#include "GeoPhysicalConstants.h"
#include "GeoPhysicsProjectHandle.h"

#include "CompoundLithology.h"
#include "ElementFunctions.h"
#include "DomainProperty.h"

DataAccess::Mining::BasinTemperatureGradientCalculator::BasinTemperatureGradientCalculator ( const DomainPropertyCollection*            collection,
                                                                                             DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                                             const Interface::Snapshot*                 snapshot,
                                                                                             const Interface::Property*                 property ) :
   DomainFormationProperty ( collection, propertyManager, snapshot, property )
{

   m_temperature = 0;
   m_initialised = false;
}

bool DataAccess::Mining::BasinTemperatureGradientCalculator::initialise () {

   if ( not m_initialised ) {
      m_temperature = getPropertyCollection ()->getDomainProperty ( "Temperature", getPropertyManager ());

      if ( m_temperature != 0 ) {
         m_initialised = true;
      } else {
         m_initialised = false;
      }

   }

   return m_initialised;
}

double DataAccess::Mining::BasinTemperatureGradientCalculator::compute ( const ElementPosition& position ) const {

   if ( not m_initialised or position.getFormation () == 0 or not position.isValidElement ()) {
      return Interface::DefaultUndefinedMapValue;
   }

   const GeoPhysics::ProjectHandle* projectHandle = dynamic_cast<const GeoPhysics::ProjectHandle*>(getProjectHandle ());

   double surfaceTemperature;
   double surfaceDepth;
   double positionTemperature;
   double positionDepth;
   double temperatureGradient;
   double time = getSnapshot ()->getTime ();
   unsigned int i;
   

   double surfaceDepths [ 4 ];
   double surfaceTemperatures [ 4 ];
   double basisFunctions [ 4 ];

   const double xi  = position.getReferencePoint ().x ();
   const double eta = position.getReferencePoint ().y ();

   basisFunctions [ 0 ] = 0.25 * ( 1.0 - xi ) * ( 1.0 - eta );
   basisFunctions [ 1 ] = 0.25 * ( 1.0 + xi ) * ( 1.0 - eta );
   basisFunctions [ 2 ] = 0.25 * ( 1.0 + xi ) * ( 1.0 + eta );
   basisFunctions [ 3 ] = 0.25 * ( 1.0 - xi ) * ( 1.0 + eta );

   surfaceDepths [ 0 ] = projectHandle->getSeaBottomDepth ( position.getI (),     position.getJ (),     time );
   surfaceDepths [ 1 ] = projectHandle->getSeaBottomDepth ( position.getI () + 1, position.getJ (),     time );
   surfaceDepths [ 2 ] = projectHandle->getSeaBottomDepth ( position.getI () + 1, position.getJ () + 1, time );
   surfaceDepths [ 3 ] = projectHandle->getSeaBottomDepth ( position.getI (),     position.getJ () + 1, time );

   surfaceTemperatures [ 0 ] = projectHandle->getSeaBottomTemperature ( position.getI (),     position.getJ (),     time );
   surfaceTemperatures [ 1 ] = projectHandle->getSeaBottomTemperature ( position.getI () + 1, position.getJ (),     time );
   surfaceTemperatures [ 2 ] = projectHandle->getSeaBottomTemperature ( position.getI () + 1, position.getJ () + 1, time );
   surfaceTemperatures [ 3 ] = projectHandle->getSeaBottomTemperature ( position.getI (),     position.getJ () + 1, time );

   surfaceTemperature = 0.0;
   surfaceDepth = 0.0;

   for ( i = 0; i < 4; ++i ) {
      surfaceTemperature += surfaceTemperatures [ i ] * basisFunctions [ i ];
      surfaceDepth += surfaceDepths [ i ] * basisFunctions [ i ];
   }

   positionDepth = position.getActualPoint ()( 2 );
   positionTemperature = m_temperature->compute ( position );

   if ( positionDepth != surfaceDepth ) {
      temperatureGradient = 1000.0 * ( positionTemperature - surfaceTemperature ) / ( positionDepth - surfaceDepth );
   } else {
      temperatureGradient = 0.0;
   }

   return temperatureGradient;
}

DataAccess::Mining::DomainProperty* DataAccess::Mining::BasinTemperatureGradientCalculatorAllocator::allocate ( const DomainPropertyCollection*            collection,
                                                                                                                DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                                                                const Interface::Snapshot*                 snapshot,
                                                                                                                const Interface::Property*                 property ) const {
   return new BasinTemperatureGradientCalculator ( collection, propertyManager, snapshot, property );
}


