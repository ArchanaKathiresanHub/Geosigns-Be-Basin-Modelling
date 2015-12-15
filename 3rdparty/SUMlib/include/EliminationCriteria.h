// Copyright 2015, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_ELIMINATION_CRITERIA_H
#define SUMLIB_ELIMINATION_CRITERIA_H

#include "SUMlib.h"

#include <boost/optional.hpp>


namespace SUMlib
{

class ProxyBuilder;

class INTERFACE_SUMLIB EliminationCriterion
{
   public:
      virtual ~EliminationCriterion();

      /// Operator used to decide whether an active variable should be eliminated.
      /// @return If no variable should be eliminated, the method returns boost::none. Otherwise, returns the
      /// index in the list of active variables corresponding to the variable which is to be eliminated.
      virtual boost::optional<unsigned int> operator()(ProxyBuilder const& cases) = 0;
};

class INTERFACE_SUMLIB NoElimination : public EliminationCriterion
{
   public:
      virtual ~NoElimination();

      /// @returns the value boost::none
      boost::optional<unsigned int> operator()(ProxyBuilder const& cases);
};

class INTERFACE_SUMLIB BonferroniElimination : public EliminationCriterion
{
   public:
      /// Creates a Bonferroni elimination criterion with a custom multiplier
      BonferroniElimination(double multiplier);

      virtual ~BonferroniElimination();

      /// Based on the Bonferroni criterion, decides whether an active variable should be eliminated.
      /// @return If no variable should be eliminated, the method returns boost::none. Otherwise, returns the
      /// index in the list of active variables corresponding to the variable which is to be eliminated.
      boost::optional<unsigned int> operator()(ProxyBuilder const& cases);
   private:
      double m_multiplier;
};

}
#endif
