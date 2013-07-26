#include "Interface/PermafrostEvent.h"
#include "Interface/ProjectHandle.h"

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include <sstream>
#include <iostream>
using namespace std;

DataAccess::Interface::PermafrostEvent::PermafrostEvent ( ProjectHandle* projectHandle, database::Record * record ) : DAObject ( projectHandle, record ) {

   m_permafrost   = ( database::getPermafrostInd ( record ) == 1 );
   m_pressureTerm = ( database::getPressureTerm  ( record ) == 1 );
   m_salinityTerm = ( database::getSalinityTerm  ( record ) == 1 );
}

DataAccess::Interface::PermafrostEvent::~PermafrostEvent () {
}

bool DataAccess::Interface::PermafrostEvent::getPermafrost () const {
   return m_permafrost;
}

bool DataAccess::Interface::PermafrostEvent::getPressureTerm () const {
   return m_pressureTerm;
}

bool DataAccess::Interface::PermafrostEvent::getSalinityTerm () const {
   return m_salinityTerm;
}

void DataAccess::Interface::PermafrostEvent::setPermafrost( const bool aPermafrost ) 
{
   m_permafrost = aPermafrost;
}

std::string DataAccess::Interface::PermafrostEvent::image () const {

   std::stringstream buffer;

   buffer << "Permafrost event." << std::endl;
   buffer << "  pressure term is : " << ( getPressureTerm() ? "on" : "off" ) << std::endl;
   buffer << "  salinity term is : " << ( getSalinityTerm() ? "on" : "off" ) << std::endl;

   return buffer.str ();
}
