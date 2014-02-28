// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_STEPPROPOSER_H
#define SUMLIB_STEPPROPOSER_H

#include <vector>

#include "SUMlib.h"

namespace SUMlib {

class RandomGenerator;

/**
*  Creates new parameter proposals for an existing parameter sample.
*
*  Creates new parameter proposals for an existing parameter sample,
*  taking into account the parameter factor bounds. These bounds
*  cause the transition probability from p1 to p2 to be different
*  from the transition probability p2 to p1. The transition probability
*  ratio as needed by the MCMC acception criterium is calculated as
*  well. The aim is to use a truncated normal transition distribution,
*  and this is approximated by using a number of steps using a
*  uniform rectangular transition distribution, trusting on the central
*  limit theorem. The average step size is hence determined by the
*  number of steps and the rectangular distribution sizes.
*/

class INTERFACE_SUMLIB_DEBUG StepProposer
{
   public:

      ///  Constructor.
      StepProposer( RandomGenerator& trg,
                    const size_t pSize,
                    std::vector<double> const& min,
                    std::vector<double> const& max,
                    unsigned int nbSteps  );

      /// Destructor
      virtual ~StepProposer();

      /**
       *  For an existing sample of parameters, generate a new
       *  proposed sample using several steps of small rectangular
       *  distribution shifts.
       *  @param p On input the existing parameter sample, on output
       *     the proposed sample.
       *  @param tr The ratio of the probabilities of the transition
       *     to and from.
       */
      void proposeStep(
            std::vector<std::vector<double> >& p,    // In: p1, Out: p2
            std::vector<double>& tr ) const;

      /**
       *  For an existing parameters, randomly generate a new
       *  proposed parameter using several steps of small rectangular
       *  distribution shifts.
       *  @param p On input the existing parameter, on output
       *     the proposed parameter.
       *  @param tr The ratio of the probabilities of the transition
       *     to and from.
       */
      void proposeRandomStep(
            std::vector<double>& p,    // In: p1, Out: p2
            double& tr ) const;

      /**
       *  For an existing parameters, generate a new
       *  proposed parameter using a Tornado perturbation.
       *  @param p On input the existing parameter, on output
       *     the proposed parameter.
       *  @return The proposed Tornado perturbation
       */
      unsigned int proposeTornadoStep( std::vector<double>& p, unsigned int step,
            unsigned int nbSteps, unsigned int lastStep ) const;

      /**
       *  To avoid too high or too low acception rates in the
       *  MCMC module, one can adapt the size of the proposal
       *  steps using this function. The ideal acceptance rate
       *  should be between 23% and 45% according to Roberts,
       *  Gelman and Gilks (1994).
       */
      void adaptStepSize( double acceptanceRate );

      const std::vector<double>& min() const { return m_min; }
      const std::vector<double>& max() const { return m_max; }
      const std::vector<double>& dp() const { return m_dp; }
      void setStepSize(std::vector<double>const& dp );

   private:
      /**
       * Check dimensions of the various member arrays
       */
      void check() const;
      /**
       *  The random generation to provide random shifts.
       */
      RandomGenerator& m_rg;
      /**
       *  Lower bounds on the parameter factor values.
       */
      std::vector<double> m_min;
      /**
       *  Upper bounds on the parameter factor values.
       */
      std::vector<double> m_max;
      /**
       *  Size of the rectangular transition distribution for the
       *  parameter factor values.
       */

      // Bounds for the step sizes
      std::vector<double> m_minDp;
      std::vector<double> m_maxDp;

       /**
       *  Size of the rectangular transition distribution for the
       *  parameter factor values.
       */
      std::vector<double> m_dp;
      /**
       *  The number of steps using a shift from the rectangular
       *  transition distribution. The sum of all approved shifts
       *  is the final proposal shift.
       */
      size_t m_nbSteps;

}; // class StepProposer

} // namespace SUMlib

#endif // SUMLIB_STEPPROPOSER_H
