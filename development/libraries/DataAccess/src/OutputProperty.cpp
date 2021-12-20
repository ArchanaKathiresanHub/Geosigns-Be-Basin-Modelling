#include <assert.h>

#include <iostream>
#include <sstream>
using namespace std;

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "OutputProperty.h"
#include "ProjectHandle.h"

using namespace DataAccess;
using namespace Interface;

OutputProperty::OutputProperty (ProjectHandle& projectHandle, database::Record * record) :
   DAObject (projectHandle, record), m_name ( database::getPropertyName (m_record))
{
   const string& outputStr = database::getOutputOption ( record );

   if ( outputStr == "None" ) {
      m_option = NO_OUTPUT;
   } else if ( outputStr == "SedimentsOnly" ) {
      m_option = SEDIMENTS_ONLY_OUTPUT;
   } else if ( outputStr == "SourceRockOnly" ) {
      m_option = SOURCE_ROCK_ONLY_OUTPUT;
   } else if ( outputStr == "SedimentsPlusBasement" ) {
      m_option = SEDIMENTS_AND_BASEMENT_OUTPUT;
   } else {
      // Error.
   }
}

OutputProperty::OutputProperty (ProjectHandle& projectHandle, const PropertyOutputOption option, const std::string& name ) :
   DAObject ( projectHandle, 0 ), m_name ( name ), m_option ( option ) {
}

OutputProperty::~OutputProperty (void)
{
}

const std::string & OutputProperty::getName (void) const {
   return m_name;
}

PropertyOutputOption OutputProperty::getOption (void) const {
   return m_option;
}

void OutputProperty::setOption ( const PropertyOutputOption newOption ) {
   m_option = newOption;
}

void OutputProperty::printOn (ostream & ostr) const {
   string str;
   asString (str);
   ostr << str;
}

void OutputProperty::asString (string & str) const
{
   ostringstream buf;

   buf << "OutputProperty: ";
   buf << " name = " << getName ();
   buf << ", output-option = ";

   switch ( getOption ()) {
     case NO_OUTPUT                     : buf << "None"; break;
     case SOURCE_ROCK_ONLY_OUTPUT       : buf << "SourceRockOnly"; break;
     case SEDIMENTS_ONLY_OUTPUT         : buf << "SedimentsOnly"; break;
     case SEDIMENTS_AND_BASEMENT_OUTPUT : buf << "SedimentsPlusBasement"; break;
   default : buf << "Incorrectly defined output option";
   }

   buf << endl;

   str = buf.str ();
}

