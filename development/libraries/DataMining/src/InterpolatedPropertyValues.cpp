#include "InterpolatedPropertyValues.h"

#include <sstream>
#include <iomanip>

DataAccess::Mining::InterpolatedPropertyValues::InterpolatedPropertyValues () {
}

DataAccess::Mining::InterpolatedPropertyValues::~InterpolatedPropertyValues () {
   clear ();
}

void DataAccess::Mining::InterpolatedPropertyValues::clear () {
   m_evaluations.clear ();
   m_evaluationPosition.clear ();
}

void DataAccess::Mining::InterpolatedPropertyValues::setElement ( const ElementPosition& elem ) {
   m_evaluationPosition = elem;
}

double DataAccess::Mining::InterpolatedPropertyValues::operator ()( const Interface::Property* property ) const {

   PropertyEvaluation::const_iterator propIter = m_evaluations.find ( property );

   if ( propIter != m_evaluations.end ()) {
      return propIter->second;
   } else {
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }

}

void DataAccess::Mining::InterpolatedPropertyValues::setValue ( const Interface::Property* property,
                                                                const double               value ) {

   if ( property == 0 ) {
      // Do not add null properties.
      return;
   }

   // If value already exists then it will be over-written!
   m_evaluations [ property ] = value;
}

bool DataAccess::Mining::InterpolatedPropertyValues::contains ( const Interface::Property* property ) const {

   PropertyEvaluation::const_iterator propIter = m_evaluations.find ( property );

   return propIter != m_evaluations.end ();
}


std::string DataAccess::Mining::InterpolatedPropertyValues::image () const {

   std::stringstream buffer;
   PropertyEvaluation::const_iterator propIter;

   buffer << " Values: {";

   buffer.precision ( 5 );
//    buffer.flags (  );
   buffer.flags ( std::ios::scientific | std::ios::left);

   for ( propIter = m_evaluations.begin (); propIter != m_evaluations.end (); ++propIter ) {
      buffer << "( " << std::setw ( 30 ) << propIter->first->getName () << " = " << std::setw ( 12 ) << propIter->second << " )";
   }

   buffer << "}";

   return buffer.str ();
}
