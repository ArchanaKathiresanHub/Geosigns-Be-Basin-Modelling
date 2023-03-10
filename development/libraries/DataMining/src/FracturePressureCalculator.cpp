#include "FracturePressureCalculator.h"

#include "GeoPhysicsProjectHandle.h"
#include "GeoPhysicsFormation.h"
#include "GeoPhysicsFluidType.h"
#include "CompoundLithology.h"
#include "FracturePressureCalculator.h"

DataAccess::Mining::FracturePressureCalculator::FracturePressureCalculator ( const DomainPropertyCollection*            collection,
                                                                             DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                             const Interface::Snapshot*                 snapshot,
                                                                             const Interface::Property*                 property ) :
   DomainFormationProperty ( collection, propertyManager, snapshot, property )
{
   m_hydrostaticPressure = 0;
   m_lithostaticPressure = 0;
   // m_porePressure = 0;
   m_initialised = false;
}

bool DataAccess::Mining::FracturePressureCalculator::initialise () {

   if ( not m_initialised ) {

      m_hydrostaticPressure = getPropertyCollection ()->getDomainProperty ( "HydroStaticPressure", getPropertyManager ());
      m_lithostaticPressure = getPropertyCollection ()->getDomainProperty ( "LithoStaticPressure", getPropertyManager ());
      // m_porePressure = getPropertyCollection ()->getDomainProperty ( "Pressure", getPropertyManager ());

      if ( m_hydrostaticPressure != 0 and m_lithostaticPressure != 0 ) { //and m_porePressure != 0) {
         m_initialised = true;
      } else {
         m_initialised = false;
      }

   }

   return m_initialised;
}

double DataAccess::Mining::FracturePressureCalculator::compute ( const ElementPosition& position ) const {

   if ( not m_initialised ) {
      return Interface::DefaultUndefinedMapValue;
   }

   const Interface::Formation* formation = position.getFormation ();
   const GeoPhysics::GeoPhysicsFormation* geoForm = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation );
   const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>( geoForm->getFluidType ());

   if ( formation == 0 or fluid == 0 ) {
      return Interface::DefaultUndefinedMapValue;
   }

   double depth;
   double hydrostaticPressure;
   double lithostaticPressure;
   // double porePressure;
   double seaBottomTemperature;
   double seaBottomDepth;
   double fracturePressure;

   // porePressure = m_porePressure->compute ( position );
   hydrostaticPressure = m_hydrostaticPressure->compute ( position );
   lithostaticPressure = m_lithostaticPressure->compute ( position );

   if ( //porePressure != Interface::DefaultUndefinedMapValue and
        hydrostaticPressure != Interface::DefaultUndefinedMapValue and
        lithostaticPressure != Interface::DefaultUndefinedMapValue ) {

      const GeoPhysics::ProjectHandle* projectHandle = dynamic_cast<const GeoPhysics::ProjectHandle*>(getProjectHandle ());

      const GeoPhysics::CompoundLithology* lithology = geoForm->getCompoundLithology ( position.getI (), position.getJ ());

      seaBottomDepth = projectHandle->getSeaBottomDepth ( position.getI (), position.getJ (), getSnapshot ()->getTime ());
      seaBottomTemperature = projectHandle->getSeaBottomTemperature ( position.getI (), position.getJ (), getSnapshot ()->getTime ());

      depth = position.getActualPoint ()( 2 );
      fracturePressure = projectHandle->getFracturePressureCalculator ().fracturePressure ( lithology, fluid, seaBottomTemperature, seaBottomDepth, depth, hydrostaticPressure, lithostaticPressure );

   } else {
      fracturePressure = Interface::DefaultUndefinedMapValue;
   }

   return fracturePressure;
}

DataAccess::Mining::DomainProperty* DataAccess::Mining::FracturePressureCalculatorAllocator::allocate ( const DomainPropertyCollection*            collection,
                                                                                                        DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                                                        const Interface::Snapshot*                 snapshot,
                                                                                                        const Interface::Property*                 property ) const {
   return new FracturePressureCalculator ( collection, propertyManager, snapshot, property );
}
