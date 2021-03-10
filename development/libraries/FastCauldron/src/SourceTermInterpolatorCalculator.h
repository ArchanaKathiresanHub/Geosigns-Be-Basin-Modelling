//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef FASTCAULDRON__SOURCE_TERM_INTERPOLATOR_CALCULATOR_H
#define FASTCAULDRON__SOURCE_TERM_INTERPOLATOR_CALCULATOR_H

#include "LayerElement.h"
#include "VectorInterpolator.h"
#include "BoundaryId.h"
#include "PVTCalculator.h"

/// \brief Computes the interpolator for the source term.
///
///
class SourceTermInterpolatorCalculator {

public :
   SourceTermInterpolatorCalculator () {}
   /// \brief The number of components an element has.
   static const unsigned int NumberOfEquations = NumberOfPVTComponents;

   typedef VectorInterpolator<NumberOfEquations> MultiInterpolator;

   typedef MultiInterpolator::CoefficientArray CoefficientArray;


   /// \brief Compute the interpolators for each face of the element.
   void compute ( const LayerElement&  element, const CoefficientArray&  xs, MultiInterpolator& interpolator ) const;

};

#endif // FASTCAULDRON__SOURCE_TERM_INTERPOLATOR_CALCULATOR_H
