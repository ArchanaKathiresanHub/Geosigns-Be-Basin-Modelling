//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef FASTCAUDLRON__ELEMENT_ALWAYS_ACTIVE_PREDICATE__H
#define FASTCAUDLRON__ELEMENT_ALWAYS_ACTIVE_PREDICATE__H

#include "LayerElement.h"

/// \brief Indicate that the element is always active.
///
/// This is of little use in the real simulation, but is a useful aid in debugging.
class ElementAlwaysActivePredicate : public ElementActivityPredicate {

public :

   virtual isActive ( const LayerElement& element ) const;

};

inline ElementAlwaysActivePredicate::isActive ( const LayerElement& element ) const {
   return true;
}


#endif // FASTCAUDLRON__ELEMENT_ALWAYS_ACTIVE_PREDICATE__H
