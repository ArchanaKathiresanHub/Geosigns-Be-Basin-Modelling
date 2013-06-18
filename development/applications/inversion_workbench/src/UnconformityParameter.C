#include "UnconformityParameter.h"
#include "project.h"

#include <iostream>
#include <string>

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


