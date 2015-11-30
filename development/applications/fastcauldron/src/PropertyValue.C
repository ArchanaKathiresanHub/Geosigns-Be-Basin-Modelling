#include "PropertyValue.h"

// Fastcauldron
#include "FastcauldronSimulator.h"
#include "Property.h"

// DataAccess
#include "Interface/Formation.h"
#include "Interface/Surface.h"
#include "Interface/MapWriter.h"
#include "Interface/Snapshot.h"


PropertyValue::PropertyValue ( Interface::ProjectHandle * projectHandle,
                               database::Record * record,
                               const string & name, 
                               const Interface::Property * property,
                               const Interface::Snapshot * snapshot,
                               const Interface::Reservoir * reservoir,
                               const Interface::Formation * formation,
                               const Interface::Surface * surface, 
                               const Interface::PropertyStorage storage ) :
   Interface::PropertyValue ( projectHandle, record, name, property, snapshot, reservoir, formation, surface, storage ) {
   m_allowOutput = true;
}

bool PropertyValue::toBeSaved () const {
   return m_allowOutput and outputIsRequested ();
}


void PropertyValue::allowOutput ( const bool output ) {
   m_allowOutput = output;
}


bool PropertyValue::outputIsRequested () const {

   bool requested = false;

   if ( m_formation == 0 and m_surface == 0 or not m_allowOutput ) {
      requested = false;
   } else {
      const Property* property = (Property*)(getProperty ());

      if ( property == 0 or property->getOption () == Interface::NO_OUTPUT ) {
         requested = false;
      } else if ( property->getOption () == Interface::SEDIMENTS_AND_BASEMENT_OUTPUT ) {
         requested = true;
      } else {
         Interface::PropertyOutputOption option = property->getOption ();
         const Interface::Formation* theFormation;

         if ( m_formation != 0 ) {
            theFormation = m_formation;
         } else {

            if (  m_surface->getBottomFormation () != 0 and m_surface->getBottomFormation ()->kind () == Interface::SEDIMENT_FORMATION ) {
               theFormation = m_surface->getBottomFormation ();
            } else {
               theFormation = m_surface->getTopFormation ();
            }

         }

         switch ( option ) {
           case Interface::SOURCE_ROCK_ONLY_OUTPUT : requested = theFormation->isSourceRock (); break; 
           case Interface::SEDIMENTS_ONLY_OUTPUT   : requested = theFormation->kind () == Interface::SEDIMENT_FORMATION; break;
           case Interface::SEDIMENTS_AND_BASEMENT_OUTPUT : requested = true; break;
           default : requested = false;
         }

      }
      
   }

   return requested;
}

bool PropertyValue::saveMapToFile ( Interface::MapWriter & mapWriter ) {

   if ( toBeSaved ()) {
      return Interface::PropertyValue::saveMapToFile ( mapWriter );
   } else {
      return true;
   }

}

bool PropertyValue::saveVolumeToFile ( Interface::MapWriter & mapWriter ) {

   if ( outputIsRequested ()) {
      return Interface::PropertyValue::saveVolumeToFile ( mapWriter );
   } else {
      return true;
   } 
}

bool PropertyValue::savePrimaryVolumeToFile ( Interface::MapWriter & mapWriter ) {

   bool status = true;

   if ( FastcauldronSimulator::getInstance ().isPrimary() and outputIsRequested ()) { 
      status = Interface::PropertyValue::savePrimaryVolumeToFile ( mapWriter );
   }
   return status;
}

database::Record * PropertyValue::createTimeIoRecord (database::Table * timeIoTbl, Interface::ModellingMode theMode) {

//    cout << " property-value: " << getName () << "  " 
//         << ( getFormation () == 0 ? " NO-FORMATION " : getFormation ()->getName ()) << "  "
//         << ( getSurface () == 0 ? " NO-SURFACE " : getSurface ()->getName ()) << "  "
//         << getSnapshot ()->getTime () 
//         << endl;

   database::Record* record = 0;
   if ( FastcauldronSimulator::getInstance ().getModellingMode () == Interface::MODE3D ) {

      std::string surfaceName = ( m_surface == 0 ? "" : m_surface->getName ());
      std::string formationName = ( m_formation == 0 ? "" : m_formation->getName ());

      record = FastcauldronSimulator::getInstance ().findTimeIoRecord ( timeIoTbl, getName (), m_snapshot->getTime (), surfaceName, formationName );

//       cout << " PropertyValue::createTimeIoRecord " << (unsigned long)( record ) << endl;

      if ( record == 0 ) {
         record = Interface::PropertyValue::createTimeIoRecord ( timeIoTbl, theMode );
      } else {
         setRecord ( record );
      }

   } else {
      record = Interface::PropertyValue::createTimeIoRecord ( timeIoTbl, theMode );
   }
   return record;
}
