#include "Biodegrade.h"

#include <iostream>

#include <math.h>

namespace migration {

// Biodegradation processing: compute the weight lost
void Biodegrade::calculate(const double& timeInterval, const double& T_C, 
   const double* input, double* lost) const
{
   assert(timeInterval > 0.0);

   if (T_C > m_maxBioTemp) 
      return;

   for (int compIdx = 0; compIdx < m_bioConsts.size(); compIdx++)
   {
      // Calculate the weight reduction factors by biodegradation:
      double bigT = pow((m_maxBioTemp - T_C) / m_maxBioTemp, m_bioConsts[compIdx]);
      double degradeFactor = pow ((1.0 - bigT), (timeInterval * m_timeFactor));

      lost[compIdx] = (1.0 - degradeFactor) * input[compIdx];
      if (degradeFactor < 0.0)
      {
         cerr << "Warning: trying to biodegrade more of component " << compIdx << " than is in the trap, degradeFactor is " << degradeFactor << endl;
	      lost[compIdx] = input[compIdx];
      }
      if (degradeFactor > 1.0)
      {
         cerr << "Warning: trying to inverse biodegrade component " << compIdx << ", degradeFactor is " << degradeFactor << endl;
	      lost[compIdx] = 0;
      }
   }
}

} // namespace migration
