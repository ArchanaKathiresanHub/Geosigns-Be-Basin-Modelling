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
   {}
 
   Biodegrade(const double maxBioTemp, const DataAccess::Interface::BioConsts bioConsts, const double timeFactor) :
      m_maxBioTemp(maxBioTemp),
      m_bioConsts(bioConsts),
      m_timeFactor(timeFactor)
   {}

   void calculate(const double& timeInterval, const double& T_C, 
      const double* input, double* lost) const;

   /*!
   * \brief Function allowing to get back the upper temperature limit at which biodegradation can occur.
   * \return the upper temperature limit at which biodegradation can occur (in °C). Set by default to 80°C.
   */
   const double& maxBioTemp() const { return m_maxBioTemp; }

   /*!
   * \brief Function allowing to retrieve all the bioconstants for biodegradation.
   * \return BioConst for each component in the folowing order:
   * Asphaltene, Resins, C15+ Aro, C15+ Sat, C6-14 Aro, C6-14 Sat, C5, C4, C3, C2, C1, COx, N2
   */
   const DataAccess::Interface::BioConsts& bioConsts() const { return m_bioConsts; }

   /*!
   * \brief Function allowing to get back the timeFactor for biodegradation.
   * \return timeFactor, a user-tuneable rate scalar for all component class (in 1/Myr). Set by default to 0.5.
   */
   const double& timeFactor() const { return m_timeFactor; }
};

} // namespace migration

#endif
