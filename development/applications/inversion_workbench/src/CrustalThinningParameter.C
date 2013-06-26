#include <string>
#include <vector>
#include <iostream>

#include "CrustalThinningParameter.h"
#include "project.h"


CrustalThinningParameter::CrustalThinningParameter(double startTime, double duration, double ratio)
   : m_startTime(startTime)
   , m_duration(duration)
   , m_ratio(ratio)                        
{
}

void CrustalThinningParameter::print(std::ostream & output)
{ 
   output << "Crustal thinning event: " 
      << "start time = " << m_startTime << " Ma, "
      << "duration = " << m_duration << " million years, "
      << "ratio = " << m_ratio ;
}

void CrustalThinningParameter::changeParameter(Project & project)
{
   project.addCrustThicknessThinningEvent(m_startTime, m_duration, m_ratio);
}

