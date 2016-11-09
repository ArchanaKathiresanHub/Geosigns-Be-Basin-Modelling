//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <iostream>
#include <math.h>

#include "Biodegrade.h"

namespace migration {

void Biodegrade::calculate(const double timeInterval, const double temperatureTrap, 
   const double* input, double* lost) const
{
   assert(timeInterval > 0.0); // the timeInterval (= time between the two snapshots) needs to be positive

   if (temperatureTrap > m_maxBioTemp) // condition for biodegradation to happen
      return;
   if (temperatureTrap < 0.0) // assess that biodegradation doesn't happen below 0°C (due to the living bugs)
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

      // No biodegradation allow for CO2 
      // (this loop can be usefull because even if the biodegradation coefficient of CO2 is set to "1.0",
      // with extreme values, for the temperature factor for instance, there is a risk that CO2 can still be slightly biodegraded)
      if (compIdx == 11 )
      {
         lost[compIdx] = 0.0;
         continue;
      }
           
      // Calculate the weight reduction factors by biodegradation:
      double T = pow((m_maxBioTemp - temperatureTrap) / m_maxBioTemp, m_bioConsts[compIdx]);
      double degradeFactor = pow ((1.0 - T), (timeInterval * m_timeFactor));

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

// legacy biodegradation
// biodegradation processing. pre and pos mean before and after biodegradation is applied
void Biodegrade::calculateLegacy (const double timeInterval, const double T_C,
   const double* input, double* lost) const
{
   assert (timeInterval > 0.0);

   if (T_C > m_maxBioTemp)
      return;

   for (int compIdx = 0; compIdx < m_bioConsts.size (); compIdx++)
   {
      // Calculate the weight reduction factors by biodegradation:
      double bigT = pow ((m_maxBioTemp - T_C) / m_maxBioTemp, m_bioConsts[compIdx]);
      double degradeFactor = pow ((1.0 - bigT), (timeInterval * m_timeFactor));

      lost[compIdx] = (1.0 - degradeFactor) * input[compIdx];
      if (degradeFactor < 0)
      {
         cerr << "Warning: all component " << compIdx << " is biodegraded. The degradeFactor is " << degradeFactor << endl;
         lost[compIdx] = input[compIdx];
      }
      if (degradeFactor > 1)
      {
         cerr << "Warning: for component " << compIdx << " the degradeFactor is " << degradeFactor <<", no biodegradation occurs "<< endl;
         lost[compIdx] = 0;
      }
   }
}

} // namespace migration
