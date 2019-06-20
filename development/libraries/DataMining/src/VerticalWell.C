#include "VerticalWell.h"

#include "Interface.h"

DataAccess::Mining::VerticalWell::VerticalWell ( const std::string& name ) : CauldronWell ( name )
{
}

DataAccess::Mining::VerticalWell::~VerticalWell () {
}


void DataAccess::Mining::VerticalWell::setTop ( const Numerics::Point& top ) {
   m_top = top;
}

void DataAccess::Mining::VerticalWell::setLength ( const double length ) {
   m_bottom ( 0 ) = m_top ( 0 );
   m_bottom ( 1 ) = m_top ( 1 );
   m_bottom ( 2 ) = m_top ( 2 ) + length;
   m_length = length;
}

Numerics::Point DataAccess::Mining::VerticalWell::getTop () const {
   return m_top;
}

Numerics::Point DataAccess::Mining::VerticalWell::getLocation ( const double depthAlongHole ) const {

   Numerics::Point location;

   if ( depthAlongHole < 0.0 or depthAlongHole > m_length ) {
      location ( 0 ) = Interface::DefaultUndefinedMapValue;
      location ( 1 ) = Interface::DefaultUndefinedMapValue;
      location ( 2 ) = Interface::DefaultUndefinedMapValue;
   } else {
      location ( 0 ) = m_top ( 0 );
      location ( 1 ) = m_top ( 1 );
      location ( 2 ) = m_top ( 2 ) + depthAlongHole;
   }

   return location;
}

double DataAccess::Mining::VerticalWell::getLength () const {
   return m_length;
}

Numerics::Point DataAccess::Mining::VerticalWell::getBottom () const {
   return m_bottom;
}
