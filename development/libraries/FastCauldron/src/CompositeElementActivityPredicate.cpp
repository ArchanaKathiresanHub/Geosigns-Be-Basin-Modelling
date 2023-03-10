//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "CompositeElementActivityPredicate.h"

CompositeElementActivityPredicate::CompositeElementActivityPredicate () {}

CompositeElementActivityPredicate::CompositeElementActivityPredicate ( const CompositeElementActivityPredicate& composite ) : 
   m_activityComposition ( composite.m_activityComposition )
{}

CompositeElementActivityPredicate::~CompositeElementActivityPredicate () {
   m_activityComposition.clear ();
}

CompositeElementActivityPredicate& CompositeElementActivityPredicate::compose ( const ElementActivityPredicatePtr& part ) {

   if ( part != 0 ) {
      // Only add non-null pointers to the list.
      m_activityComposition.push_back ( part );
   }

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
