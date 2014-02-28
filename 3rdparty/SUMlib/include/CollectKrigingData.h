// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_COLLECTKRIGINGDATA_H
#define SUMLIB_COLLECTKRIGINGDATA_H

#include <map>

#include "BaseTypes.h"
#include "KrigingData.h"
#include "SUMlib.h"

namespace SUMlib
{
/// @class CollectKrigingData stores Kriging data for each combination of the
/// categorical variables.
///
/// The class can handle parameters that have both categorical and integer/real
/// valued parameter variables. The categorical part is stripped off internally
/// and the doubles are cast to unsigned int.
///
/// The parameter space is assumed to be prepared.
///
/// Important note: this class has become obsolete.
/// TODO: remove this class.

class INTERFACE_SUMLIB CollectKrigingData
{
   public:

      /// Default constructor
      CollectKrigingData();

      /// Destructor
      virtual ~CollectKrigingData();

      /// Constructor: can only be used if no categorical parameter elements exist.
      ///
      /// @param [in]  parSet         Parameter set.
      CollectKrigingData(
            ParameterSet const&        parSet
            );

      /// Constructor for the generic case.
      /// @param [in]  parSet         Parameter set.
      /// @param [in]  numCategorical number of categorical parameter elements.
      CollectKrigingData(
            ParameterSet const&        parSet,
            unsigned int               numCategorical
            );

      /// Stores the Kriging data in a map.
      /// @param [in]  parSet         Parameter set.
      /// @param [in]  numCategorical number of categorical parameter elements.
      void initialise(
            ParameterSet const&        parSet,
            unsigned int               numCategorical
            );

   private:

      /// For each categorical combination the corresponding Kriging data is collected
      typedef std::map<CategoricalCombination, KrigingData*> KrigingDataMap;

      /// Kriging data is stored for each combination of the categorical values.
      KrigingDataMap          m_KrigingDataMap;
};

} // namespace SUMlib

#endif // SUMLIB_COLLECTKRIGINGDATA_H
