// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_HYBRIDMC_H
#define SUMLIB_HYBRIDMC_H

#include <vector>

#include "ExpDesignBase.h"
#include "SUMlib.h"


namespace SUMlib {

class Case;
class ParameterBounds;
class RandomGenerator;

/**
 * HybridMC creates a space-filling design which can be augmented by the user.
 * The "van der Corput" sequence in base 2 is followed for the first dimension.
 * Independent random permutations of this sequence are used for the other dimensions.
 *
 * The design initially contains values between 0 and 1, according to the "van der Corput" sequence.
 * After that, all design values are scaled to [-1, +1] according to literature.
 * Only later in getCaseSet, they will be mapped according to the original ranges.
 */
class INTERFACE_SUMLIB HybridMC : public ExpDesignBase
{
   // Random permutations based on rg.uniformRandom().
   RandomGenerator *rg;

   // The number of old runs = the number of already available runs.
   size_t m_nbOldRuns;

   // The number of extra runs = the number of rows in m_design.
   size_t m_nbNewRuns;

public:

   /**
    * Constructor of a Hybrid Quasi-Monte Carlo design for all types of parameters.
    */
   HybridMC( std::vector<bool> const& selPar, unsigned int numOrdFactors,
             unsigned int nbOldRuns, unsigned int nbNewRuns );

   virtual ~HybridMC();

   /**
    * Generate the Hybrid Quasi-Monte Carlo design.
    */
   virtual void generate();

   /**
    * getNbOfCases overrides the implementation in ExpDesignBase.
    */
   virtual unsigned int getNbOfCases( ParameterBounds const& bounds,
                                      bool replicate = false ) const;

   /*
    * Write the design to a string
    */
   virtual std::string toString() const;

private:

   virtual void getCaseSetImpl( ParameterBounds const& bounds, Case const& center,
                                bool replicate, std::vector<Case>& caseSet ) const;
};

} // namespace SUMlib

#endif
