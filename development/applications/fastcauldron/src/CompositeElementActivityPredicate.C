#include "CompositeElementActivityPredicate.h"

CompositeElementActivityPredicate::CompositeElementActivityPredicate () {}

CompositeElementActivityPredicate::CompositeElementActivityPredicate ( const CompositeElementActivityPredicate& composite ) : 
   m_activityComposition ( composite.m_activityComposition )
{}

CompositeElementActivityPredicate::~CompositeElementActivityPredicate () {
   m_activityComposition.clear ();
}

CompositeElementActivityPredicate& CompositeElementActivityPredicate::compose ( const ElementActivityPredicatePtr& part ) {
   m_activityComposition.push_back ( part );
   return *this;
}

bool CompositeElementActivityPredicate::isActive ( const LayerElement& element ) const {

   // Initialised to true only if there are items on the predicate list.
   // Otherwise set to false so that no element is considered to be active.
   bool result = m_activityComposition.size () > 0;

   for ( size_t i = 0; i < m_activityComposition.size (); ++i ) {
      result = result and m_activityComposition [ i ]->isActive ( element );
   }

   return result;
}
