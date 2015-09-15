//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef FASTCAUDLRON__SEDIMENT_ELEMENT_ACTIVITY_PREDICATE__H
#define FASTCAUDLRON__SEDIMENT_ELEMENT_ACTIVITY_PREDICATE__H

#include "ElementActivityPredicate.h"
#include "LayerElement.h"

/// \brief Class used to determine if an element is a part of the sediments or not.
///
/// If this class is used in the composite element activity predicte then elements
/// that are a part of the sediments only. Elements in the basement (crust and mantle)
/// will designated as being in-active.
class SedimentElementActivityPredicate : public ElementActivityPredicate {

public :

   /// \brief Active only if element forms a part of the sediments and not the basement.
   virtual isActive ( const LayerElement& element ) const;

};

inline SedimentElementActivityPredicate::isActive ( const LayerElement& element ) const {
   return element.getFormation () != 0 and element.getFormation ()->isSediment ();
}

#endif // FASTCAUDLRON__SEDIMENT_ELEMENT_ACTIVITY_PREDICATE__H
