#include <sstream>
#include "OutputPropertyMap.h"

OutputPropertyMap::OutputPropertyMap ( const PropertyList         propertyName,
                                             LayerProps*          formation,
                                       const Interface::Surface*  surface,
                                       const Interface::Snapshot* snapshot ) :
   m_propertyName ( propertyName ), m_formation ( formation ), m_surface ( surface ), m_snapshot ( snapshot ) {

   m_properties.clear ();
   m_values.clear ();
   m_propertyMaps.clear ();
}

bool OutputPropertyMap::isRequired ( const unsigned int position ) const {
   return m_values [ position ]->outputIsRequested ();
}

bool OutputPropertyMap::anyIsRequired () const {

   unsigned int i;

   for ( i = 0; i < m_values.size (); ++i ) {

      if ( m_values [ i ]->outputIsRequested ()) {
         return true;
      }

   }

   return false;
}

bool OutputPropertyMap::matches ( const std::string&         propertyName,
                                  const LayerProps*          formation,
                                  const Interface::Surface*  surface,
                                  const Interface::Snapshot* snapshot ) const {

   // Initialised with the name of the property (this may encompass several sub-properties)
   // E.g. FluidVelocity is made up of FluidVelocityX, FluidVelocityY and FluidVelocityZ.
   bool propertyMatches = propertyName == PropertyName [ m_propertyName ];
   unsigned int i;

   assert ( formation != 0 or surface != 0 );

   for ( i = 0; i < m_properties.size (); ++i ) {
      propertyMatches = propertyMatches or m_properties [ i ]->getName () == propertyName;
   }

   return propertyMatches and ( formation == 0 ? true : formation == m_formation ) and ( surface == 0 ? true : surface == m_surface ) and m_snapshot == snapshot;
}

double OutputPropertyMap::operator ()( const unsigned int i, const unsigned int j, const unsigned int position ) const {
   return getMapValue ( i, j );
}

double OutputPropertyMap::getMapValue ( const unsigned int i, const unsigned int j, const unsigned int position ) const {
   return m_propertyMaps [ position ]->getValue ( i, j );
}

double OutputPropertyMap::getVolumeValue ( const unsigned int i, const unsigned int j, const unsigned int k, const unsigned int position ) const {
   return m_propertyMaps [ position ]->getValue ( i, j, k );
}

unsigned int OutputPropertyMap::numberOfProperties () const {
   return m_properties.size ();
}

PropertyList OutputPropertyMap::getPropertyName () const {
   return m_propertyName;
}

std::string OutputPropertyMap::image () const {

   std::ostringstream buffer;
   unsigned int i;

   if ( m_formation == 0 ) {
      buffer << " NO FORMATION, " ;
   } else {
      buffer << m_formation->layername << ", ";
   }

   if ( m_surface == 0 ) {
      buffer << " NO SURFACE, " ;
   } else {
      buffer << m_surface->getName () << ", ";
   }

   buffer << m_snapshot->getTime () << ", properties [ ";

   for ( i = 0; i < m_properties.size (); ++i ) {
      buffer << m_properties [ i ]->getName ();

      if ( i < m_properties.size () - 1 ) {
         buffer << ", ";
      }

   }

   buffer << " ]";

   return buffer.str ();
}
