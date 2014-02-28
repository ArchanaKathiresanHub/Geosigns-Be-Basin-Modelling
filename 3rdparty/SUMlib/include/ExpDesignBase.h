// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_EXPDESIGNBASE_H
#define SUMLIB_EXPDESIGNBASE_H

#include "BaseTypes.h"
#include "ExpDesign.h"
#include "SUMlib.h"

namespace SUMlib {

class Case;
class ParameterBounds;

/// @class ExpDesignBase is the base class for experimental designs.
class INTERFACE_SUMLIB ExpDesignBase : public ExpDesign
{
   public:
      ExpDesignBase( std::vector<bool> const& selectedFactors, unsigned int numOrdinalFactors );
      virtual ~ExpDesignBase();

      unsigned int numFactors() const;
      unsigned int numSelFactors() const;
      unsigned int numOrdFactors() const;
      unsigned int numSelOrdFactors() const;
      std::vector<bool> const& selFactors() const;

      virtual unsigned int getNbOfCases( ParameterBounds const& bounds,
                                         bool replicate = true ) const;
      virtual void getCaseSet( ParameterBounds const& bounds, Case const& center,
                               bool replicate, std::vector<Case>& caseSet ) const;
      virtual void setAddCenterPoint( bool addCenterPoint );
      virtual bool getAddCenterPoint() const;

      virtual void setAddMirror( bool addMirror );
      virtual bool getAddMirror() const;

   protected:
      typedef std::vector<std::vector<double> > DesignMatrix;
      DesignMatrix m_design;

      void checkBoundsAndCenter( ParameterBounds const& bounds, Case const& center ) const;
      void checkDesignDimensions( unsigned int n, DesignMatrix const& design ) const;
      int convertOrdToDis( ParameterBounds const& bounds, unsigned int idx, double value ) const;

      // Insert base case values in the design matrix for unselected parameters.
      void expandDesign( unsigned int n, DesignMatrix const& designIn, DesignMatrix& designOut );

      // Create ordinal design in the original dimensions without the categorical extension.
      void createOrdDesign( ParameterBounds const& bounds, Case const& center,
                            DesignMatrix& conDes, std::vector<std::vector<int> >& disDes ) const;

      // Create categorical design from m_design; only used by HybridMC and FaureDesign.
      void createCatDesign( ParameterBounds const& bounds,
                            std::vector<std::vector<unsigned int> >& catDes ) const;

      void fillCaseSet( ParameterBounds const& bounds, Case const& center,
                        bool replicate, std::vector<Case>& caseSet ) const;
      void fillCaseSet( ParameterBounds const& bounds, Case const& center,
                        std::vector<Case>& caseSet ) const;

   private: //methods
      virtual void getCaseSetImpl( ParameterBounds const& bounds, Case const& center,
                           bool replicate, std::vector<Case>& caseSet ) const = 0;
      void calcCatCombinations( ParameterBounds const& bounds,
                                std::vector<std::vector<unsigned int> >& catCombiSet ) const;

   private: //data
      unsigned int m_numFactors, m_numOrdFactors;
      std::vector<bool> const m_selFactors;
      unsigned int m_numSelFactors, m_numSelOrdFactors;
};

} // namespace SUMlib

#endif // SUMLIB_EXPDESIGNBASE_H
