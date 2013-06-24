#include "BasementParameter.h"
#include "project.h"

#include <iostream>

BasementParameter::BasementParameter( const std::string & name, double value)
   : m_name( name )
   , m_value( value )
{
}

void BasementParameter::print(std::ostream & output)
{ 
   output << "Basement property '" << m_name << "' = " << m_value ;
}

void BasementParameter::changeParameter(Project & project)
{
   project.setBasementProperty(m_name, m_value);
}


