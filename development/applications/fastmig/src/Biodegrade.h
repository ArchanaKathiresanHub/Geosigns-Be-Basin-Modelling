//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_BIODEGRADE_H_
#define _MIGRATION_BIODEGRADE_H_

#include "Interface/BiodegradationParameters.h"

namespace migration {

class Biodegrade
{

public:

   /*!
   * \brief Constructor used for the biodegradation parameters within Fastmig
   */
   Biodegrade(const DataAccess::Interface::BiodegradationParameters* biodegradationparameters):
      m_maxBioTemp(biodegradationparameters->maxBioTemp()),
      m_bioConsts(biodegradationparameters->bioConsts()),
      m_timeFactor(biodegradationparameters->timeFactor()),
      m_bioRate(biodegradationparameters->bioRate()),
      m_pasteurizationInd(biodegradationparameters->pasteurizationInd())
   {}

   /*!
   * \brief Constructor used for the biodegradation parameters in unit tests only
   */
   Biodegrade(const double maxBioTemp, const DataAccess::Interface::BioConsts bioConsts, const double timeFactor, const double bioRate = 0.3, const bool pasteurizationInd = 0) :
      m_maxBioTemp(maxBioTemp),
      m_bioConsts(bioConsts),
      m_timeFactor(timeFactor),
      m_bioRate(bioRate),
      m_pasteurizationInd(pasteurizationInd)
   {}

   /*!
   * \brief Biodegradation processing: compute the weight lost
   */
   void calculate(const double timeInterval, const double temperatureTrap,
      const double* input, double* lost) const;   

   /*!
   * \brief Get back the upper temperature limit at which biodegradation can occur.
   * \return the upper temperature limit at which biodegradation can occur (in °C). Set by default to 80°C.
   */
   const double& maxBioTemp() const { return m_maxBioTemp; }

   /*!
   * \brief Retrieve all the bioconstants for biodegradation.
   * \return BioConst for each component in the folowing order:
   * Asphaltene, Resins, C15+ Aro, C15+ Sat, C6-14 Aro, C6-14 Sat, C5, C4, C3, C2, C1, COx, N2
   */
   const DataAccess::Interface::BioConsts& bioConsts() const { return m_bioConsts; }

   /*!
   * \brief Get back the timeFactor for biodegradation.
   * \return timeFactor, a user-tuneable rate scalar for all component class (in 1/Myr). Set by default to 0.5.
   */
   const double& timeFactor() const { return m_timeFactor; }

   /*!
   * \brief Get the biodegradation rate.
   * \details This rate helps to define the thickness affected by biodegradation above OWC
   * \return bioRate, a user-tuneable rate scalar which sets how the biodegradation is expending above the OWC (in m/Myr). Set by default to 0.3.
   */
   const double& bioRate() const { return m_bioRate; }

   /*!
   * \brief Get back the pasteurization status
   * \details Describe if the biodegradation process should take into account the pasteurization (bool = 1) or not (bool =0).
   * \return The pasteurization status (1 = On, 0 = Off)
   */
   const bool& pasteurizationInd() const { return m_pasteurizationInd; }


private:

   double const m_maxBioTemp;
   DataAccess::Interface::BioConsts const m_bioConsts;
   double const m_timeFactor;
   double const m_bioRate;
   bool const m_pasteurizationInd;
};

} // namespace migration

#endif
