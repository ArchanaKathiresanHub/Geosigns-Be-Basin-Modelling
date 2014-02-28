// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_OPTIMISEDLHD_H
#define SUMLIB_OPTIMISEDLHD_H

#include <vector>

#include "ExpDesignBase.h"
#include "SUMlib.h"


namespace SUMlib {

class Case;
class ParameterBounds;
class RandomGenerator;

/**
 * OptimisedLHD creates an optimised Latin Hypercube design containing N points.
 * The collection {0, 1,..., N-1} is taken for the first dimension (i.e. first coordinate).
 * Random permutations of this collection are used for the other dimensions,
 * after which these permutations are (locally) optimised.
 *
 * The objective function to be minimised is the sum of all squared inverse distances.
 *
 * Subsequently, all design values are scaled to [-1, +1] according to literature.
 * Only later in getCaseSet, they will be mapped according to the original ranges.
 */
class INTERFACE_SUMLIB OptimisedLHD : public ExpDesignBase
{
   // Random permutations based on rg.uniformRandom().
   RandomGenerator *rg;

   // The number of runs = the number of rows in m_design.
   size_t N;

   // The number of selected factors = the number of active dimensions.
   size_t K;

public:

   /**
    * Constructor of the optimised Latin Hypercube design for all types of parameters.
    */
   OptimisedLHD( std::vector<bool> const& selPar, unsigned int numOrdFactors,
                 unsigned int nbRuns );

   virtual ~OptimisedLHD();

   /**
    * Generate the optimised Latin Hypercube design.
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

   void createRandomLHD( std::vector<size_t>& coordinates,
                  std::vector<std::vector<size_t> >& design ) const;

   void calcDistances( std::vector<std::vector<size_t> > const& design,
                  std::vector<std::vector<size_t> >& distances ) const;

   double calcObjFunc( std::vector<std::vector<size_t> > const& distances ) const;

   double proposeSwap( size_t ix, size_t jx, size_t kx, double objF,
                  std::vector<std::vector<size_t> > const& design,
                  std::vector<std::vector<size_t> > const& distances,
                  std::vector<size_t>& dDist ) const;

   double calcObjTerm( size_t distance ) const;

   void adjustDistances( size_t ix, size_t jx, std::vector<size_t> const& dDist,
                  std::vector<std::vector<size_t> >& distances ) const;
};

} // namespace SUMlib

#endif
