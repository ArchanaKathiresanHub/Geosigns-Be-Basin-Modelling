//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef FASTCAUDLRON__ELEMENT_THICKNESS_ACTIVITY_PREDICATE__H
#define FASTCAUDLRON__ELEMENT_THICKNESS_ACTIVITY_PREDICATE__H

#include "ElementActivityPredicate.h"
#include "LayerElement.h"

/// \brief The activity of the element is defined by its thickness.
///
/// If all four vertical segments of the element are zero-thickness then
/// the element is designated in-active.
class ElementThicknessActivityPredicate : public ElementActivityPredicate {

public :

   /// \bref Determine the activity of the element.
   virtual bool isActive ( const LayerElement& element ) const;

};

inline bool ElementThicknessActivityPredicate::isActive ( const LayerElement& element ) const {
   return element.isActive ();
}

#endif // FASTCAUDLRON__ELEMENT_THICKNESS_ACTIVITY_PREDICATE__H
