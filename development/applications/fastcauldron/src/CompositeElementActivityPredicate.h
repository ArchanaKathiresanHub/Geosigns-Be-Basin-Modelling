//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef FASTCAUDLRON__COMPOSITE_ELEMENT_ACTIVITY_PREDICATE__H
#define FASTCAUDLRON__COMPOSITE_ELEMENT_ACTIVITY_PREDICATE__H

// STL library.
#include <vector>

// Access to fastcauldron application code.
#include "ElementActivityPredicate.h"
#include "LayerElement.h"

/// \brief A composition of element activity predicates.
///
/// The result of each of the individual predicates is combined using a logical conjuction operation (Boolean AND).
/// An example of use may be for the pressure solver, only elements which have a
/// non-zero thickness and with a non-zero porosity are included in the domain. So
/// both types of activity predicate must be added.
class CompositeElementActivityPredicate {

   /// \typedef ElementActivityPredicatePtrArray
   /// \brief A one dimensional array of (smart-)pointers to ElementActivityPredicate.
   typedef std::vector<ElementActivityPredicatePtr> ElementActivityPredicatePtrArray;

public :

   /// \brief Default constructor.
   CompositeElementActivityPredicate ();

   /// \brief A copy constructor.
   CompositeElementActivityPredicate ( const CompositeElementActivityPredicate& composite  );

   ~CompositeElementActivityPredicate ();


   /// \brief Add an individual ElementActivityPredicate to the composition.
   CompositeElementActivityPredicate& compose ( const ElementActivityPredicatePtr& part );

   /// \brief Determine if the element is active for the particular action required.
   bool isActive ( const LayerElement& element ) const;

private :

   /// \brief The list of individual element activity predicates.
   ElementActivityPredicatePtrArray m_activityComposition;

};

#endif // FASTCAUDLRON__COMPOSITE_ELEMENT_ACTIVITY_PREDICATE__H
