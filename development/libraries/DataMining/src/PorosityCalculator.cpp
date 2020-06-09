#include "PorosityCalculator.h"

#include "ProjectHandle.h"
#include "RunParameters.h"

#include "GeoPhysicsFormation.h"
#include "CompoundLithology.h"

DataAccess::Mining::PorosityCalculator::PorosityCalculator ( const DomainPropertyCollection*            collection,
                                                             DerivedProperties::DerivedPropertyManager& propertyManager,
                                                             const Interface::Snapshot*                 snapshot,
                                                             const Interface::Property*                 property ) :
   DomainFormationProperty ( collection, propertyManager, snapshot, property )
{
   m_ves = 0;
   m_maxVes = 0;
   m_chemicalCompaction = 0;
   m_chemicalCompactionRequired = getPropertyCollection ()->getProjectHandle ()->getRunParameters ()->getChemicalCompaction ();

   m_initialised = false;
}

bool DataAccess::Mining::PorosityCalculator::initialise () {

   if ( not m_initialised ) {
      m_ves = getPropertyCollection ()->getDomainProperty ( "Ves", getPropertyManager ());
      m_maxVes = getPropertyCollection ()->getDomainProperty ( "MaxVes", getPropertyManager ());

      if ( m_chemicalCompactionRequired ) {
         m_chemicalCompaction = getPropertyCollection ()->getDomainProperty ( "ChemicalCompaction", getPropertyManager ());
      } else {
         m_chemicalCompaction = 0;
      }

      if ( m_ves != 0 and m_maxVes != 0 and (( m_chemicalCompactionRequired and m_chemicalCompaction != 0 ) or not m_chemicalCompactionRequired )) {
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
   double chemicalCompaction;
   double porosity;

   ves = m_ves->compute ( position );
   maxVes = m_maxVes->compute ( position );

   const Interface::Formation* formation = position.getFormation ();
   const GeoPhysics::GeoPhysicsFormation* geoForm = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation );

   bool layerRequiresChemicalCompaction = m_chemicalCompactionRequired and ( geoForm != 0 and geoForm->hasChemicalCompaction ());

   if ( layerRequiresChemicalCompaction ) {
      chemicalCompaction = m_chemicalCompaction->compute ( position );
   } else {
      chemicalCompaction = Interface::DefaultUndefinedMapValue;
   }

   if ( ves != Interface::DefaultUndefinedMapValue and maxVes != Interface::DefaultUndefinedMapValue and
        (( layerRequiresChemicalCompaction and chemicalCompaction != Interface::DefaultUndefinedMapValue ) or not layerRequiresChemicalCompaction )) {

      const GeoPhysics::CompoundLithology* lithology = geoForm->getCompoundLithology ( position.getI (), position.getJ ());

      porosity = 100.0 * lithology->porosity ( ves, maxVes, layerRequiresChemicalCompaction, chemicalCompaction );
   } else {
      porosity = Interface::DefaultUndefinedMapValue;
   }

   return porosity;
}

DataAccess::Mining::DomainProperty* DataAccess::Mining::PorosityCalculatorAllocator::allocate ( const DomainPropertyCollection*            collection,
                                                                                                DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                                                const Interface::Snapshot*                 snapshot,
                                                                                                const Interface::Property*                 property ) const {
   return new PorosityCalculator ( collection, propertyManager, snapshot, property );
}
