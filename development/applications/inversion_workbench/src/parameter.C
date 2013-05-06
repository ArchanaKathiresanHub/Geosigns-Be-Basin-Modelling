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
  std::cout << "Basement property '" << m_name << "' = " << m_value << std::endl;
}

void BasementParameter::Change_parameter(Project & project)
{
  project.setBasementProperty(m_name, m_value);
}



CrustalThinningParameter::CrustalThinningParameter( std::vector< ThicknessAtTime> & series)
: m_series(series)
{
}

void CrustalThinningParameter::print()
{ 
  std::cout << "Crustal thinning time series : " ;
  for (unsigned i = 0; i < m_series.size(); ++i)
    std::cout << '(' << m_series[i].first << " Ma, " << m_series[i].second << " m) ";
  std::cout << std::endl;
}

void CrustalThinningParameter::Change_parameter(Project & project)
{
  project.setCrustThickness(m_series);
}

