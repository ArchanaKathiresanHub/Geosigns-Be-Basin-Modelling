#include "BasementParameter.h"
#include "project.h"

#include <iostream>
#include <cassert>

BasementParameter::BasementParameter( const std::string & name, double value)
   : m_name( name )
   , m_value( value )
{
}

void BasementParameter::print(std::ostream & output)
{ 
   output << "Basement property '" << m_name << "' = " << m_value ;
}

void BasementParameter::changeParameter( Project & project )
{
   project.setBasementProperty(m_name, m_value);
}

std::vector<double> BasementParameter::toDblVector() const 
{
   return std::vector<double>( 1, m_value );
}

void BasementParameter::fromDblVector( const std::vector<double> & prms )
{
   assert( prms.size() == 1 );
   m_value = prms[0];
}
