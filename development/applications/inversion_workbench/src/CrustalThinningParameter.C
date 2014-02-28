#include "CrustalThinningParameter.h"
#include "project.h"

#include <string>
#include <vector>
#include <iostream>
#include <cassert>


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

std::vector<double> CrustalThinningParameter::toDblVector() const 
{
   std::vector<double> prms(3);
   
   prms[0] = m_startTime;
   prms[1] = m_duration;
   prms[2] = m_ratio;

   return prms;
}

void CrustalThinningParameter::fromDblVector( const std::vector<double> & prms )
{
   assert( prms.size() == 3 );

   m_startTime = prms[0];
   m_duration  = prms[1];
   m_ratio     = prms[2];
}
