#include "CauldronGridDescription.h"

#include <sstream>
#include <iomanip>

using namespace GeoPhysics;

GeoPhysics::CauldronGridDescription::CauldronGridDescription () {

  nrI = 0;
  nrJ = 0;

  originI = 0.0;
  originJ = 0.0;

  deltaI = -1.0;
  deltaJ = -1.0;

#if 0
  originI = CAULDRONIBSNULLVALUE;
  originJ = CAULDRONIBSNULLVALUE;

  deltaI = CAULDRONIBSNULLVALUE;
  deltaJ = CAULDRONIBSNULLVALUE;
#endif

}


void GeoPhysics::CauldronGridDescription::print ( std::ostream& o ) const {

   o << std::setw ( 5 ) << nrI << std::setw ( 20 ) << originI << std::setw ( 20 ) << deltaI << std::endl;
   o << std::setw ( 5 ) << nrJ << std::setw ( 20 ) << originJ << std::setw ( 20 ) << deltaJ << std::endl;

}


std::string GeoPhysics::CauldronGridDescription::image () const {

   std::stringstream buffer;

   buffer << " Size   : " << std::setw ( 15 ) <<     nrI << "  " << std::setw ( 15 ) <<     nrJ << std::endl;
   buffer << " Origin : " << std::setw ( 15 ) << originI << "  " << std::setw ( 15 ) << originJ << std::endl;
   buffer << " Delta  : " << std::setw ( 15 ) <<  deltaI << "  " << std::setw ( 15 ) <<  deltaJ << std::endl;

   return buffer.str ();
}
