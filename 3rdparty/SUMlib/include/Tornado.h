// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_TORNADO_H
#define SUMLIB_TORNADO_H

#include "ExpDesignBase.h"
#include "SUMlib.h"

namespace SUMlib {

/**
 * A Tornado experimental design is created for a given number of factors.
 * A center point is added to the design and serves as the base case.
 * Specified for each factor: a low and high case.
 */
class INTERFACE_SUMLIB Tornado : public ExpDesignBase
{

public:
   /**
    * Constructor.
    * Creates a Tornado design for nbOfOrdPar factors and generates the design data.
    * @param [in] selPar         flag indicating which parameters are selected
    * @param [in] nbOfOrdPar     the number of ordinal parameters
    */
   Tornado( std::vector<bool> const& selPar, unsigned int nbOfOrdPar );

   virtual ~Tornado();

   /**
    * Generate the Tornado design.
    */
   virtual void generate();

   virtual std::string toString() const;

private:

   virtual void getCaseSetImpl( ParameterBounds const& bounds, Case const& center,
                                bool replicate, std::vector<Case>& caseSet ) const;
};

} // namespace SUMlib

#endif
