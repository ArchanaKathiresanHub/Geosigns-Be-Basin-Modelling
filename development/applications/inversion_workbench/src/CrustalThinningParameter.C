#include <string>
#include <vector>
#include <iostream>

#include "CrustalThinningParameter.h"
#include "project.h"


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

void CrustalThinningParameter::changeParameter(Project & project)
{
   project.setCrustThickness(m_series);
}

