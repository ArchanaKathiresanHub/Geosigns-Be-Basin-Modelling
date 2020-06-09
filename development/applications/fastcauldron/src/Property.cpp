#include "Property.h"

#include "Property.h"

#include <iostream>
#include <sstream>

Property::Property ( Interface::ProjectHandle& projectHandle, database::Record * record,
                     const string & userName, const string & cauldronName,
                     const string & unit, const Interface::PropertyType type,
                     const DataModel::PropertyAttribute attr, const DataModel::PropertyOutputAttribute attrOut ) :
   Interface::Property ( projectHandle, record, userName, cauldronName, unit, type, attr, attrOut ) {

   m_outputProperty = 0;

}

Interface::PropertyOutputOption Property::getOption () const {

   Interface::PropertyOutputOption result;

   if ( m_outputProperty != 0 ) {
      result = m_outputProperty->getOption ();
   } else {
      result = Interface::NO_OUTPUT;
   }

   return result;
}

const Interface::OutputProperty* Property::getOutputProperty () const {
   return m_outputProperty;
}

void Property::setOutputProperty ( const Interface::OutputProperty* outputProperty ) {
   m_outputProperty = outputProperty;
}

void Property::asString ( string& str ) const {

   std::ostringstream buffer;

   std::string propertyPrefix;

   this->Interface::Property::asString ( propertyPrefix );

   buffer << propertyPrefix;

   if ( m_outputProperty != 0 ) {
      buffer << ", time-filter name = " << m_outputProperty->getName ();
   } else {
      buffer << ", time-filter name = UNKNOWN";
   }

   switch ( getOption ()) {
     case Interface::NO_OUTPUT                     : buffer << ", output-option = None"; break;
     case Interface::SOURCE_ROCK_ONLY_OUTPUT       : buffer << ", output-option = SourceRockOnly"; break;
     case Interface::SEDIMENTS_ONLY_OUTPUT         : buffer << ", output-option = SedimentsOnly"; break;
     case Interface::SEDIMENTS_AND_BASEMENT_OUTPUT : buffer << ", output-option = SedimentsPlusBasement"; break;
     default : buffer << ", output-option is incorrectly defined output option";
   }

   str = buffer.str ();
}
