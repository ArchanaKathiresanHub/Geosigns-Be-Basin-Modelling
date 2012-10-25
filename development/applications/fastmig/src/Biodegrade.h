#ifndef _MIGRATION_BIODEGRADE_H_
#define _MIGRATION_BIODEGRADE_H_

#include "Interface/BiodegradationParameters.h"
#include "Interface/BioConsts.h"

namespace migration {

class Biodegrade
{
private:

   double m_maxBioTemp;
   DataAccess::Interface::BioConsts m_bioConsts;
   double m_timeFactor;

public:

   Biodegrade(const DataAccess::Interface::BiodegradationParameters* 
         biodegradationparameters):
      m_maxBioTemp(biodegradationparameters->maxBioTemp()),
      m_bioConsts(biodegradationparameters->bioConsts()),
      m_timeFactor(biodegradationparameters->timeFactor())
   {
   }

   void calculate(const double& timeInterval, const double& T_C, 
      const double* input, double* lost) const;

   const double& maxBioTemp() const { return m_maxBioTemp; }
   const DataAccess::Interface::BioConsts& bioConsts() const { return m_bioConsts; }
   const double& timeFactor() const { return m_timeFactor; }
};

} // namespace migration

#endif
