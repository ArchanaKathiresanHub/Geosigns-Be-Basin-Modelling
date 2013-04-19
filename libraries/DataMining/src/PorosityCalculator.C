#include "PorosityCalculator.h"

#include "GeoPhysicsFormation.h"
#include "CompoundLithology.h"

DataAccess::Mining::PorosityCalculator::PorosityCalculator ( const DomainPropertyCollection*  collection,
                                                             const Interface::Snapshot* snapshot,
                                                             const Interface::Property* property ) :
   DomainFormationProperty ( collection, snapshot, property )
{
   m_ves = 0;
   m_maxVes = 0;
   m_initialised = false;
}

bool DataAccess::Mining::PorosityCalculator::initialise () {

   if ( not m_initialised ) {
      m_ves = getPropertyCollection ()->getDomainProperty ( "Ves" );
      m_maxVes = getPropertyCollection ()->getDomainProperty ( "MaxVes" );

      if ( m_ves != 0 and m_maxVes != 0 ) {
         m_initialised = true;
      } else {
         m_initialised = false;
      }

   }

   return m_initialised;
}

double DataAccess::Mining::PorosityCalculator::compute ( const ElementPosition& position ) const {

   if ( not m_initialised ) {
      return Interface::DefaultUndefinedMapValue;
   }

   double ves;
   double maxVes;
   double porosity;

   ves = m_ves->compute ( position );
   maxVes = m_maxVes->compute ( position );

   if ( ves != Interface::DefaultUndefinedMapValue and maxVes != Interface::DefaultUndefinedMapValue ) {

      const Interface::Formation* formation = position.getFormation ();
      const GeoPhysics::Formation* geoForm = dynamic_cast<const GeoPhysics::Formation*>( formation );
      const GeoPhysics::CompoundLithology* lithology = geoForm->getCompoundLithology ( position.getI (), position.getJ ());

      porosity = 100.0 * lithology->porosity ( ves, maxVes, false, 0.0 );
   } else {
      porosity = Interface::DefaultUndefinedMapValue;
   }

   return porosity;
}

DataAccess::Mining::DomainProperty* DataAccess::Mining::PorosityCalculatorAllocator::allocate ( const DomainPropertyCollection*  collection,
                                                                                                const Interface::Snapshot* snapshot,
                                                                                                const Interface::Property* property ) const {
   return new PorosityCalculator ( collection, snapshot, property );
}
