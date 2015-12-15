// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_FACTDESIGN_H
#define SUMLIB_FACTDESIGN_H

#include <vector>

#include "ExpDesignBase.h"
#include "SUMlib.h"

namespace SUMlib {

class DesignGenerator;

/**
 * AliasingData describes a single alias of an experimental Design.
 *
 * It consists of a number of integers. The first element (index 0) is the
 * intended sign (+1 or - 1) of the alias. The following elements are (column
 * indices + 1) of the alias expression. The last element is the (column index
 * + 1) of the factor to be aliased.
 * For example: [-1,1,2,4] will replace factor x4 with -x1*x2 which for the
 * design [x1, x2, x3, x4=x12, x13, x23]. This aliasses the effect -x12 with
 * x4.
 */
typedef std::vector<int> AliasingData;

/**
 * FactDesign creates a fractional factorial experimental design for a given
 * number of factors.
 *
 * It allows aliasing of factors by using design generators and can add
 * face centered/circumscribes/inscribed star points.
 *
 * @todo The return types are not documented in the member functions.
 */
class INTERFACE_SUMLIB FactDesign : public ExpDesignBase
{
   public:
      /**
       * Enumerates star point configurations.
       *
       * PLAIN indicates no star points.
       */
      enum FactDesignType {
         PLAIN,
         FACE_CENTERED,
         CIRCUMSCRIBED,
         INSCRIBED
      };

   private:

    /**
    * The type of the factorial design
    */
   FactDesignType m_type;

   /**
    * A list of all design generators, reducing the number of runs needed for
    * the design by aliasing factors.
    */
   std::vector<AliasingData> m_designGenerators;

   /**
    * The factor shifts from the centerpoint of the design.
    */
   DesignMatrix m_selDesign;

   /**
    * The generateRuns method recursively generates the shifts for a (partial)
    * full design.
    *
    * On the top-level it is called for the number of factors minus the number
    * of design generators in the design. The remaining factors are
    * subsequently added using the design generators.
    */
   void generateRuns( unsigned int level, std::vector<double>& des );

   void checkDesignGenerators();

public:
   static const int FullFactorialResolution = 999;

   /**
    * Recommended constructor!
    * Creates a fractional factorial design using specified generators.
    *
    * @param [in] selPar         Flag indicating which parameters are selected.
    * @param [in] nbOfOrdPar     Number of ordinal parameters.
    * @param [in] type           For adding star points to the design or not.
    * @param [in] gen            List of design generators.
    */
   FactDesign( std::vector<bool> const& selPar, unsigned int nbOfOrdPar,
               FactDesignType type = PLAIN,
               std::vector<AliasingData> const& gen = std::vector<AliasingData>() );

   /**
    * Constructor.
    * TODO: Can this constructor be removed?
    *
    * @param [in] selPar         Flag indicating which parameters are selected.
    * @param [in] nbOfOrdPar     Number of ordinal parameters.
    * @param [in] dg             The design generator data for this design
    * @param [in] type           For adding star points to the design or not.
    */
   FactDesign( std::vector<bool> const& selPar, unsigned int nbOfOrdPar,
               DesignGenerator const& dg, FactDesignType type );

   /**
    * Destructor.
    */
   virtual ~FactDesign();

   /**
    * return the factorial design type
    */
   FactDesignType type() const { return m_type; }

   /**
    * Generate the fractional factorial design, using design generators as
    * added by addGenerator, with the star points as indicated by type and
    * optionally a centerpoint.
    */
   virtual void generate();

   /**
    * Write the design to a string
    *
    * @return a string containing the serialized design data.
    */
   virtual std::string toString() const;

private:

   virtual void getCaseSetImpl( ParameterBounds const& bounds, Case const& center,
                                bool replicate, std::vector<Case>& caseSet ) const;
};

} // namespace SUMlib

#endif
