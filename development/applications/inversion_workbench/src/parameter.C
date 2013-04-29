#include <iostream>
#include <string>
#include <vector>
#include "parameter.h"

BasementParameter::BasementParameter( const std::string & name, double value)
{
  m_name = name;
  m_value = value;
}

void BasementParameter::print()
{ 
  std::cout << "m_name: " << m_name << ", m_value: " << m_value << std::endl;
}

void BasementParameter::Change_parameter(Project & project)
{
  project.setBasementProperty(m_name, m_value);
  std::cout << m_name << std::endl;
}



CrustalThinningParameter::CrustalThinningParameter( std::vector< ThicknessAtTime> & series)
: m_series(series)
{
}

void CrustalThinningParameter::print()
{ 
//  std::cout << "Not implemented yet. " << std::endl ;
}

void CrustalThinningParameter::Change_parameter(Project & project)
{
  project.setCrustThickness(m_series);
//  std::cout << m_name << std::endl;
}

