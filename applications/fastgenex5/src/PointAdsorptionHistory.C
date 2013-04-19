#include "PointAdsorptionHistory.h"

#include <iomanip>
#include <sstream>

#include "mangle.h"

#include "cauldronschemafuncs.h"

PointAdsorptionHistory::PointAdsorptionHistory (  AdsorptionProjectHandle* adsorptionProjectHandle,
                                                  database::Record*        record ) :
   m_adsorptionProjectHandle ( adsorptionProjectHandle )
{

   m_x = database::getXPlanePosition ( record );
   m_y = database::getYPlanePosition ( record );
   m_formationName = database::getAdsorptionFormationName ( record );
   m_mangledFormationName = utilities::mangle ( m_formationName );
}

double PointAdsorptionHistory::getX () const {
   return m_x;
}

double PointAdsorptionHistory::getY () const {
   return m_y;
}

const std::string& PointAdsorptionHistory::getFormationName () const {
   return m_formationName;
}

const std::string& PointAdsorptionHistory::getMangledFormationName () const {
   return m_mangledFormationName;
}


std::string PointAdsorptionHistory::image () const {

   std::stringstream buffer;

   buffer << " history-point: " << getFormationName () 
          << std::setw ( 15 ) << getX  () 
          << std::setw ( 15 ) << getY  ()
          << std::endl;

   return buffer.str ();
}
