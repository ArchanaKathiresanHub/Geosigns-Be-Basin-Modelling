// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_BOXBEHNKEN_H
#define SUMLIB_BOXBEHNKEN_H

#include "ExpDesignBase.h"
#include "SUMlib.h"

namespace SUMlib {

/**
 * A Box-Behnken experimental design is created for a given number of factors (> 1).
 * In practice, dealing with more than 8 factors (= 112 runs) becomes uneconomic.
 * A center point is always added to the design.
 */
class INTERFACE_SUMLIB BoxBehnken : public ExpDesignBase
{
   // The number of rows in the ordinal design.
   unsigned int m_nbRuns; //TODO: remove redundant data member

   // The design contains zeros and shifts (-1 and 1) from the center point.
   DesignMatrix m_selDesign;

   // Generates a subset of all necessary design rows, depending on the squared design radius R2.
   void addDesignRows( unsigned int R2, int i1, int i2, int i3, int i4, int i_alias );

public:

   /**
    * Constructor of a Box-Behnken design for ordinal parameters.
    * @param [in] selPar      flag indicating which parameters are selected
    * @param [in] nbOfOrdPar  the number of ordinal parameters
    */
   BoxBehnken( std::vector<bool> const& selPar, unsigned int nbOfOrdPar );

   virtual ~BoxBehnken();

   /**
    * Generate the Box-Behnken design
    */
   virtual void generate();

   /**
    * Serialize the design to string
    */
   virtual std::string toString() const;

private:

   virtual void getCaseSetImpl( ParameterBounds const& bounds, Case const& center,
                                bool replicate, std::vector<Case>& caseSet ) const;
};

} // namespace SUMlib

#endif // SUMLIB_BOXBEHNKEN_H
