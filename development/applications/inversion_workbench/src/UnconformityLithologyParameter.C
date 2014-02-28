#include "UnconformityLithologyParameter.h"
#include "project.h"

#include <stdexcept>
#include <iostream>
#include <cassert>

void UnconformityLithologyParameter::print(std::ostream & output)
{ 
   output << "UnconformityLithology property '" << m_depoFormationName << ", " << m_lithology1 << "(" << m_percentage1 << ")";
   if (m_lithology2 != "") output << ", " << m_lithology2 << "(" << m_percentage2 << ")";
   if (m_lithology3 != "") output << ", " << m_lithology3 << "(" << m_percentage3 << ")";
}

void UnconformityLithologyParameter::changeParameter(Project & project)
{
   project.setUnconformityLithologyProperty(m_depoFormationName,
	 m_lithology1, m_percentage1, m_lithology2, m_percentage2, m_lithology3, m_percentage3);
}

std::vector<double> UnconformityLithologyParameter::toDblVector() const 
{  
   throw std::runtime_error( "UnconformityLithologyParameter::toDblVector(): not implemented yet" );
   return std::vector<double>();
}

void UnconformityLithologyParameter::fromDblVector( const std::vector<double> & prms )
{
   throw std::runtime_error( "UnconformityLithologyParameter::fromDblVector(): not implemented yet" );
}
