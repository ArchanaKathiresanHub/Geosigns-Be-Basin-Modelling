#include <string>
#include <vector>
#include <iostream>

#include "CrustalThinningParameter.h"
#include "project.h"


CrustalThinningParameter::CrustalThinningParameter(double startTime, double duration, double thickness, double ratio)
   : m_startTime(startTime)
   , m_duration(duration)
   , m_thickness(thickness)
   , m_ratio(ratio)                        
{
}

void CrustalThinningParameter::print(std::ostream & output)
{ 
   output << "Crustal thinning event: " 
      << "start time = " << m_startTime << " Ma, "
      << "duration = " << m_duration << " million years, "
      << "thickness = " << m_thickness << " meters, "
      << "ratio = " << m_ratio ;
}

void CrustalThinningParameter::changeParameter(Project & project)
{
   project.setCrustThicknessThinningEvent(m_startTime, m_duration, m_thickness, m_ratio);
}

