#include "Biodegrade.h"

#include <iostream>

#include <math.h>

namespace migration {

// Biodegradation processing: compute the weight lost
void Biodegrade::calculate(const double& timeInterval, const double& T_C, 
   const double* input, double* lost) const
{
   assert(timeInterval > 0.0);

   if (T_C > m_maxBioTemp) // condition for biodegradation to happen
      return;
   if (T_C < 0.0) // assess that biodegradation doesn't happen below 0°C (due to the living bugs)
      return;
   if (m_timeFactor < 0.0) // narrow the range allowed for this parameter
   {
      cerr << "Warning: The time factor coefficient used for biodegradation is negative: " << m_timeFactor << ". No biodegradation computed" << endl;
      return;
   }   
   
   for (int compIdx = 0; compIdx < m_bioConsts.size(); compIdx++)
   {
      if (m_bioConsts[compIdx] == -199999) // then the biodegradation coefficient or the temperature factor is negative => no biodegradation
      {
         lost[compIdx] = 0.0;
         continue;
      }
      
      // Calculate the weight reduction factors by biodegradation:
      double bigT = pow((m_maxBioTemp - T_C) / m_maxBioTemp, m_bioConsts[compIdx]);
      double degradeFactor = pow ((1.0 - bigT), (timeInterval * m_timeFactor));

      if (degradeFactor < 0.0)
      {
         cerr << "Warning: trying to biodegrade more of component " << compIdx << " than is in the trap" << endl;
	      lost[compIdx] = input[compIdx];
         continue;
      }
      if (degradeFactor > 1.0)
      {
         cerr << "Warning: trying to inverse biodegrade component " << compIdx << ", and create some mass" << endl;
	      lost[compIdx] = 0.0;
         continue;
      }

      lost[compIdx] = (1.0 - degradeFactor) * input[compIdx];
   }
}

} // namespace migration
