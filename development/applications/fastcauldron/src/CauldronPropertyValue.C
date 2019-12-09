#include "CauldronPropertyValue.h"

// Fastcauldron
#include "FastcauldronSimulator.h"
#include "Property.h"

// DataAccess
#include "Formation.h"
#include "Surface.h"
#include "MapWriter.h"
#include "Snapshot.h"


CauldronPropertyValue::CauldronPropertyValue ( Interface::ProjectHandle& projectHandle,
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

bool CauldronPropertyValue::toBeSaved () const {
   return m_allowOutput and outputIsRequested ();
}


void CauldronPropertyValue::allowOutput ( const bool output ) {
   m_allowOutput = output;
}


bool CauldronPropertyValue::outputIsRequested () const
{
   bool requested = false;

   if ( m_formation == 0 and m_surface == 0 or not m_allowOutput )
   {
     return false;
   }

   const Property* property = (Property*)(getProperty ());

   if ( property == 0 )
   {
     return false;
   }

   Interface::PropertyOutputOption option = property->getOption();
   if ( option == Interface::NO_OUTPUT )
   {
     return false;
   }

   if ( option == Interface::SEDIMENTS_AND_BASEMENT_OUTPUT )
   {
     return true;
   }

   const Interface::Formation* theFormation;
   if ( m_formation != 0 )
   {
      theFormation = m_formation;
   }
   else
   {
      if ( m_surface->getBottomFormation () != 0 && m_surface->getBottomFormation ()->kind () == Interface::SEDIMENT_FORMATION )
      {
         theFormation = m_surface->getBottomFormation ();
      }
      else
      {
         theFormation = m_surface->getTopFormation ();
      }
   }

   switch ( option )
   {
     case Interface::SOURCE_ROCK_ONLY_OUTPUT : return theFormation->isSourceRock ();
     case Interface::SEDIMENTS_ONLY_OUTPUT   : return theFormation->kind () == Interface::SEDIMENT_FORMATION;
     case Interface::SEDIMENTS_AND_BASEMENT_OUTPUT : return true;
     default : return false;
   }
}

bool CauldronPropertyValue::saveMapToFile ( Interface::MapWriter & mapWriter, const bool saveAsPrimary ) {

   if ( toBeSaved ()) {
      return Interface::PropertyValue::saveMapToFile ( mapWriter, saveAsPrimary );
   } else {
      return true;
   }

}

bool CauldronPropertyValue::saveVolumeToFile ( Interface::MapWriter & mapWriter, const bool isPrimary ) {

   if ( outputIsRequested ()) {
      return Interface::PropertyValue::saveVolumeToFile ( mapWriter, isPrimary );
   } else {
      return true;
   }
}

database::Record * CauldronPropertyValue::createTimeIoRecord (database::Table * timeIoTbl, Interface::ModellingMode theMode) {

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
