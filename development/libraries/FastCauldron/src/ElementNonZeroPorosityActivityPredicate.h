//                                                                      
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef FASTCAUDLRON__ELEMENT_NON_ZERO_POROSITY_ACTIVITY_PREDICATE__H
#define FASTCAUDLRON__ELEMENT_NON_ZERO_POROSITY_ACTIVITY_PREDICATE__H

// Access to fastcauldron application code.
#include "ElementActivityPredicate.h"
#include "LayerElement.h"
#include "Lithology.h"

/// \brief The activity of the element is determined by the surface porosity.
///
/// If the element has a lithology that has a surface porosity of zero then 
/// the element is designated in-active.
class ElementNonZeroPorosityActivityPredicate : public ElementActivityPredicate {

public :

   /// \brief Determine the activity of the element.
   virtual bool isActive ( const LayerElement& element ) const;

};

inline bool ElementNonZeroPorosityActivityPredicate::isActive ( const LayerElement& element ) const {
   return element.getLithology () != 0 and element.getLithology ()->surfacePorosity () > porosityTolerance;
}


#endif // FASTCAUDLRON__ELEMENT_NON_ZERO_POROSITY_ACTIVITY_PREDICATE__H
