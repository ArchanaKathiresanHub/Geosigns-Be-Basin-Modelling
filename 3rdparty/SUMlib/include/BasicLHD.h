// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_BASICLHD_H
#define SUMLIB_BASICLHD_H

#include <vector>

#include "ExpDesignBase.h"
#include "SUMlib.h"


namespace SUMlib {

class Case;
class ParameterBounds;
class RandomGenerator;

/**
 * BasicLHD creates a basic Latin Hypercube design.
 * A collection of evenly distributed points is taken for the first dimension.
 * Independent random permutations of this collection are used for the other dimensions.
 *
 * All design values are scaled to [-1, +1] according to literature.
 * Only later in getCaseSet, they will be mapped according to the original ranges.
 */
class INTERFACE_SUMLIB BasicLHD : public ExpDesignBase
{
   // Random permutations based on rg.uniformRandom().
   RandomGenerator *rg;

   // The number of runs = the number of rows in m_design.
   size_t m_nbRuns;

public:

   /**
    * Constructor of a basic Latin Hypercube design for all types of parameters.
    */
   BasicLHD( std::vector<bool> const& selPar, unsigned int numOrdFactors,
             unsigned int nbRuns );

   virtual ~BasicLHD();

   /**
    * Generate the basic Latin Hypercube design.
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
