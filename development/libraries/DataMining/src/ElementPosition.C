#include "ElementPosition.h"

#include <sstream>

#include "Interface/Interface.h"

DataAccess::Mining::ElementPosition::ElementPosition () {
   clearReference ();

   m_actualPoint ( 0 ) = DataAccess::Interface::DefaultUndefinedMapValue;
   m_actualPoint ( 1 ) = DataAccess::Interface::DefaultUndefinedMapValue;
   m_actualPoint ( 2 ) = DataAccess::Interface::DefaultUndefinedMapValue;
   m_surface = 0;
   m_depthAlongHole = DataAccess::Interface::DefaultUndefinedMapValue;
}

DataAccess::Mining::ElementPosition::ElementPosition ( const ElementPosition& copy ) {

   // m_actualPoint    = copy.getActualPoint ();
   // m_referencePoint = copy.getReferencePoint ();

   // m_globalI = copy.getI ();
   // m_globalJ = copy.getJ ();
   // m_globalK = copy.getGlobalK ();
   // m_localK  = copy.getLocalK ();

   // m_formation = copy.getFormation ();
   // m_surface = copy.getSurface ();
   // m_depthAlongHole = copy.m_depthAlongHole;

   operator=( copy );

}

DataAccess::Mining::ElementPosition& DataAccess::Mining::ElementPosition::operator= ( const ElementPosition& rhs ) {

   m_actualPoint    = rhs.getActualPoint ();
   m_referencePoint = rhs.getReferencePoint ();

   m_globalI = rhs.getI ();
   m_globalJ = rhs.getJ ();
   m_globalK = rhs.getGlobalK ();
   m_localK  = rhs.getLocalK ();

   m_formation = rhs.getFormation ();
   m_surface = rhs.getSurface ();
   m_depthAlongHole = rhs.m_depthAlongHole;

   return *this;
}


void DataAccess::Mining::ElementPosition::clear () {

   clearReference ();

   m_actualPoint ( 0 ) = DataAccess::Interface::DefaultUndefinedMapValue;
   m_actualPoint ( 1 ) = DataAccess::Interface::DefaultUndefinedMapValue;
   m_actualPoint ( 2 ) = DataAccess::Interface::DefaultUndefinedMapValue;
   m_surface = 0;
   m_depthAlongHole = DataAccess::Interface::DefaultUndefinedMapValue;
}

void DataAccess::Mining::ElementPosition::clearReference () {

   m_referencePoint ( 0 ) = DataAccess::Interface::DefaultUndefinedMapValue;
   m_referencePoint ( 1 ) = DataAccess::Interface::DefaultUndefinedMapValue;
   m_referencePoint ( 2 ) = DataAccess::Interface::DefaultUndefinedMapValue;

   m_globalI = (unsigned int)(DataAccess::Interface::DefaultUndefinedMapValue);
   m_globalJ = (unsigned int)(DataAccess::Interface::DefaultUndefinedMapValue);
   m_globalK = (unsigned int)(DataAccess::Interface::DefaultUndefinedMapValue);
   m_localK  = (unsigned int)(DataAccess::Interface::DefaultUndefinedMapValue);

   m_formation = 0;
}


void DataAccess::Mining::ElementPosition::setMapPosition ( const unsigned int newI,
                                                           const unsigned int newJ ) {
   m_globalI = newI;
   m_globalJ = newJ;
}

void DataAccess::Mining::ElementPosition::setDepthPosition ( const unsigned int newGlobalK,
                                                             const unsigned int newLocalK ) {
   m_globalK = newGlobalK;
   m_localK  = newLocalK;
}

void DataAccess::Mining::ElementPosition::setPosition ( const unsigned int newI,
                                                        const unsigned int newJ,
                                                        const unsigned int newGlobalK,
                                                        const unsigned int newLocalK ) {

   m_globalI = newI;
   m_globalJ = newJ;
   m_globalK = newGlobalK;
   m_localK  = newLocalK;
}

void DataAccess::Mining::ElementPosition::setActualPoint ( const Numerics::Point& newPoint ) {
   m_actualPoint = newPoint;
}

void DataAccess::Mining::ElementPosition::setReferencePoint ( const Numerics::Point& newPoint ) {
   m_referencePoint = newPoint;
}

void DataAccess::Mining::ElementPosition::setDepthAlongHole ( const double dah ) {
   m_depthAlongHole = dah;
}

void DataAccess::Mining::ElementPosition::setSurface ( const DataAccess::Interface::Surface* surface ) {
   m_surface = surface;
}

void DataAccess::Mining::ElementPosition::setFormation ( const DataAccess::Interface::Formation* formation ) {
   m_formation = formation;
}

bool DataAccess::Mining::ElementPosition::isValidPlaneElement () const {
   return m_globalI != (unsigned int)(DataAccess::Interface::DefaultUndefinedMapValue) and
      m_globalJ != (unsigned int)(DataAccess::Interface::DefaultUndefinedMapValue);
}

bool DataAccess::Mining::ElementPosition::isValidElement () const {
   return isValidPlaneElement () and
      m_globalK != (unsigned int)(DataAccess::Interface::DefaultUndefinedMapValue) and
      m_localK != (unsigned int)(DataAccess::Interface::DefaultUndefinedMapValue);
}

std::string DataAccess::Mining::ElementPosition::image () const {

   std::stringstream buffer;

   if ( isValidElement ()) {
      buffer << " actual: " << getActualPoint ().image () << ", reference: " << getReferencePoint ().image () << "  " << getFormation ()->getName ();

      if ( getSurface () != 0 ) {
         buffer << "  " << getSurface ()->getName ();
      }

      buffer << " " << getI () << "  " << getJ () << "  " << getGlobalK () << "  " << getLocalK ();
   } else {
      buffer << " **** INVALID ELEMENT **** ";

      buffer << " actual: " << getActualPoint ().image ();

      if ( getSurface () != 0 ) {
         buffer << "  " << getSurface ()->getName ();
      }

   }

   return buffer.str ();
}

