#include "UnconformityParameter.h"
#include "project.h"

#include <iostream>
#include <string>
#include <cassert>

UnconformityParameter::UnconformityParameter( const std::string & depoFormationName, const std::string & name, double value) :
   m_depoFormationName (depoFormationName),
   m_name( name ),
   m_value( value )
{
}

void UnconformityParameter::print(std::ostream & output)
{ 
   output << "Unconformity property '" << m_depoFormationName << ":" << m_name << "' = " << m_value ;
}

void UnconformityParameter::changeParameter(Project & project)
{
   project.setUnconformityProperty(m_depoFormationName, m_name, m_value);
}

std::vector<double> UnconformityParameter::toDblVector() const 
{
   return std::vector<double>( 1, m_value );
}

void UnconformityParameter::fromDblVector( const std::vector<double> & prms )
{
   assert( prms.size() == 1 );
   m_value = prms[0];
}
