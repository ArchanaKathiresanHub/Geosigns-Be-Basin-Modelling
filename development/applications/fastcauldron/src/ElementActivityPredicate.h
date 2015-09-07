#ifndef FASTCAUDLRON__ELEMENT_ACTIVITY_PREDICATE__H
#define FASTCAUDLRON__ELEMENT_ACTIVITY_PREDICATE__H

// Access to the Boost library.
#include <boost/shared_ptr.hpp>

// Access to fastcauldron application code.
#include "LayerElement.h"

/// \brief Class will be used to determine if an element is active and will be used in the simulation.
class ElementActivityPredicate {

public :

   ElementActivityPredicate () {}

   virtual ~ElementActivityPredicate () {}

   /// \brief Determine if an element is active and will be used in the calculation.
   virtual bool isActive ( const LayerElement& element ) const = 0;

};

/// \typedef ElementActivityPredicatePtr
/// \brief A smart-pointer to an ElementActivityPredicate.
typedef boost::shared_ptr <ElementActivityPredicate> ElementActivityPredicatePtr;

#endif // FASTCAUDLRON__ELEMENT_ACTIVITY_PREDICATE__H
