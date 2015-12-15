// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_SCREENDESIGN_H
#define SUMLIB_SCREENDESIGN_H

#include <string>
#include <vector>

#include "DataStructureUtils.h"
#include "ExpDesignBase.h"
#include "SUMlib.h"

namespace SUMlib {

/**
 * ScreenDesign creates a Plackett-Burman experimental design for a given
 * number of factors (< 48). A center point can be added to the design.
 */
class INTERFACE_SUMLIB ScreenDesign : public ExpDesignBase
{
   typedef std::vector<std::vector<int> > HadamardMatrix;

   // Flag indicating whether to add a center point
   bool m_addCenterPoint;

   // Flag indicating whether to add the mirror design
   bool m_addMirror;

   // The number of rows in the ordinal design.
   unsigned int m_nbRuns;

   // Construct a Hadamard matrix of size 2nx2n from a Hadamard matrix of size nxn.
   HadamardMatrix Hadconstr( const HadamardMatrix &Had );

   // Construct a Plackett-Burman (sub)design from a given row.
   intMatrix PBconstr( std::vector<int>& row );

   /**
    * Add column of ones at the left of a Plackett-Burman design.
    * This results into a Hadamard matrix.
    */
   HadamardMatrix PB2Had( const intMatrix &PB );

   // Remove first column of a Hadamard matrix, resulting in a Plackett-Burman design.
   intMatrix Had2PB( const HadamardMatrix &Had );

   // Resize the rows of a variable of type intMatrix.
   void RowResize( intMatrix &A, int L );

public:

   // Constructor for a Plackett-Burman design.
   ScreenDesign( std::vector<bool> const& selPar, unsigned int nbOfOrdPar,
                 bool addCenterPoint = false, bool addMirror = false );

   virtual ~ScreenDesign();

   /**
    * Generate the Plackett-Burman design with an optional center point.
    * @param addCenterPoint indicates whether to add a center point.
    * If mirror = true, the design is extended with its mirror design.
    */
   virtual void generate();

   virtual bool getAddCenterPoint() const { return m_addCenterPoint; }
   virtual bool getAddMirror() const { return m_addMirror; }

   /**
    * Write the design to a string
    */
   virtual std::string toString() const;

private:

   virtual void getCaseSetImpl( ParameterBounds const& bounds, Case const& center,
                                bool replicate, std::vector<Case>& caseSet ) const;
};

} // namespace SUMlib

#endif // SUMLIB_SCREENDESIGN_H

