#include "AllochthonousLithologyInterpolator.h"

#include <algorithm>

#include "NumericFunctions.h"

using namespace GeoPhysics;

//------------------------------------------------------------//

bool GeoPhysics::AllochthonousLithologyInterpolator::IntervalInterpolatorEarlierThan::operator ()( const Numerics::IntervalInterpolator* interval1,
                                                                                                   const Numerics::IntervalInterpolator* interval2 ) const {
   // It is possible to use the end-age function to order the intervals,
   // the result will be the same, as long as both objects call the same function.
   // either both call start-age or both call end-age.
   return interval1->getStartAge () > interval2->getStartAge ();
}

//------------------------------------------------------------//

GeoPhysics::AllochthonousLithologyInterpolator::AllochthonousLithologyInterpolator ( CompoundLithology* initialLithology ) {
   currentInterpolator = 0;
   lithology = initialLithology;
}

//------------------------------------------------------------//

void GeoPhysics::AllochthonousLithologyInterpolator::insert ( Numerics::IntervalInterpolator* newInterpolator ) {

   intervalInterpolators.push_back ( newInterpolator );
}

//------------------------------------------------------------//

void GeoPhysics::AllochthonousLithologyInterpolator::setGridDescription ( const CauldronGridDescription& newDescription ) {
   gridDescription = newDescription;
}

//------------------------------------------------------------//

void GeoPhysics::AllochthonousLithologyInterpolator::freeze ( const int optimisationLevel ) {

   if ( intervalInterpolators.size () == 0 ) {
      return;
   }

   const double TimeStepSize =  GeoPhysics::AllochthonousLithologyInterpolator::morphingTimeStepSize ( optimisationLevel );
   const int    MaximumTimeStepCount = GeoPhysics::AllochthonousLithologyInterpolator::maximumNumberOfMorphingTimeSteps ( optimisationLevel  );
   const int    MinimumTimeStepCount = GeoPhysics::AllochthonousLithologyInterpolator::minimumNumberOfMorphingTimeSteps ( optimisationLevel  );

   std::sort ( intervalInterpolators.begin (),
               intervalInterpolators.end (),
               IntervalInterpolatorEarlierThan ());

   // Now set the time steps that are to be used when assigning the lithologies in the lithology array.

   IntervalInterpolatorSequence::const_iterator iter;
   int i;
   int numberOfTimeSteps;
   double timeStepSize;
   double timeStep;

   // Just to keep the last time.
   double lastTimeStep;

   for ( iter = intervalInterpolators.begin (); iter != intervalInterpolators.end (); ++iter ) {
      numberOfTimeSteps = NumericFunctions::Minimum ( int (((*iter)->getStartAge () - (*iter)->getEndAge ()) / TimeStepSize ),
                                                      MaximumTimeStepCount );

      numberOfTimeSteps = NumericFunctions::Maximum ( numberOfTimeSteps, MinimumTimeStepCount );
      timeStepSize = ((*iter)->getStartAge () - (*iter)->getEndAge ()) / double ( numberOfTimeSteps );

      timeStep = (*iter)->getStartAge ();

      for ( i = 0; i < numberOfTimeSteps; ++i ) {
         timestepsForMorphing.push_back ( timeStep );
         timeStep -= timeStepSize;
      }

      lastTimeStep = (*iter)->getEndAge ();
   }

   // The last time is added only once, here.
   timestepsForMorphing.push_back ( lastTimeStep );
}

//------------------------------------------------------------//

void GeoPhysics::AllochthonousLithologyInterpolator::reset () {
   currentInterpolator = intervalInterpolators [ 0 ];
}

//------------------------------------------------------------//

void GeoPhysics::AllochthonousLithologyInterpolator::setInterpolator ( const double age ) {

   size_t i;

   currentInterpolator = 0;

   // Find the interval the age lies within.
   for ( i = 0; i < intervalInterpolators.size (); ++i ) {

      // The range value look a bit odd here, but the start age os older, hence has a larger value.
      if ( NumericFunctions::inRange ( age, intervalInterpolators [ i ]->getEndAge (), intervalInterpolators [ i ]->getStartAge ())) {
         currentInterpolator = intervalInterpolators [ i ];
         break;
      }

   }

   // If an interval containing the age has not been found then set 
   // currentInterpolator to either the first or last interval-interpolator.
   if ( intervalInterpolators.size () > 0 and currentInterpolator == 0 ) {

      if ( age >= intervalInterpolators [ 0 ]->getStartAge ()) {
         currentInterpolator = intervalInterpolators [ 0 ];
      } else if ( age <= intervalInterpolators [ intervalInterpolators.size () - 1 ]->getEndAge ()) {
         currentInterpolator = intervalInterpolators [ intervalInterpolators.size () - 1 ];
      } else {
         // Error
      }

   }

   currentAgePosition = -1;

   for ( i = 0; i < timestepsForMorphing.size (); ++i ) {

      if ( age >= timestepsForMorphing [ i ]  ) {
         currentAgePosition = i;
         break;
      }

   }

   // If the currentInterpolator != 0 then the timestepsForMorphing will be a non-empty sequence of values.
   if ( currentInterpolator != 0 and currentAgePosition == -1 ) {

      if ( age > timestepsForMorphing [ 0 ]) {
         currentAgePosition = 0;
      } else if ( age < timestepsForMorphing [ timestepsForMorphing.size () - 1 ]) {
         currentAgePosition = timestepsForMorphing.size () - 1;
      }

   }

}

//------------------------------------------------------------//

CompoundLithology* GeoPhysics::AllochthonousLithologyInterpolator::operator ()( const int    i,
                                                                                const int    j,
                                                                                const double age ) {

   // Should really compare signs here, since the sign of the 'inside-salt-body'
   // may have an non-zero value, either +ve or -ve.
   if (( currentInterpolator != 0 ) and ( currentInterpolator->getStartAge () >= age ) && ((*currentInterpolator)( getPoint ( i, j, age )) > 0.0 )) {
      return lithology;
   } else {
      return 0;
   }

}

//------------------------------------------------------------//

Numerics::Point GeoPhysics::AllochthonousLithologyInterpolator::getPoint ( const int    i,
                                                                           const int    j,
                                                                           const double age ) const {

   Numerics::Point evaluationPoint;

   evaluationPoint ( 0 ) = double ( gridDescription.originI ) + double ( i ) * double ( gridDescription.deltaI );
   evaluationPoint ( 1 ) = double ( gridDescription.originJ ) + double ( j ) * double ( gridDescription.deltaJ );

   // This function should (will) not be called when currentAgePosition == -1.
   if ( currentAgePosition == -1 ) {
      evaluationPoint ( 2 ) = 0.0;
   } else {
      evaluationPoint ( 2 ) = timestepsForMorphing [ currentAgePosition ];
   }

   return evaluationPoint;
}

//------------------------------------------------------------//

GeoPhysics::AllochthonousLithologyInterpolator::const_iterator GeoPhysics::AllochthonousLithologyInterpolator::begin () const {
   return intervalInterpolators.begin ();
}

//------------------------------------------------------------//

GeoPhysics::AllochthonousLithologyInterpolator::const_iterator GeoPhysics::AllochthonousLithologyInterpolator::end () const {
   return intervalInterpolators.end ();
}

//------------------------------------------------------------//

const GeoPhysics::AllochthonousLithologyInterpolator::MorphingTimeSteps& GeoPhysics::AllochthonousLithologyInterpolator::getMorphingTimeSteps () const {
   return timestepsForMorphing;
}

//------------------------------------------------------------//

int GeoPhysics::AllochthonousLithologyInterpolator::maximumNumberOfMorphingTimeSteps ( const int optimisationLevel ) {

   // There is no basis for these numbers.
   static const int TimeSteps [ 5 ] = { 10, 10, 15, 20, 40 };

   assert ( 1 <= optimisationLevel and optimisationLevel <= 5 );

   return TimeSteps [ optimisationLevel - 1 ];
}

//------------------------------------------------------------//

int GeoPhysics::AllochthonousLithologyInterpolator::minimumNumberOfMorphingTimeSteps ( const int optimisationLevel ) {

   // There is no basis for these numbers.
   static const int TimeSteps [ 5 ] = { 2, 2, 2, 3, 4 };

   assert ( 1 <= optimisationLevel and optimisationLevel <= 5 );

   return TimeSteps [ optimisationLevel - 1 ];
}

//------------------------------------------------------------//

double GeoPhysics::AllochthonousLithologyInterpolator::morphingTimeStepSize ( const int optimisationLevel ) {

   // There is no basis for these numbers.
   static const double TimeStepSize [ 5 ] = { 1.0, 0.75, 0.5, 0.5, 0.25 };

   assert ( 1 <= optimisationLevel and optimisationLevel <= 5 );

   return TimeStepSize [ optimisationLevel - 1 ];
}

//------------------------------------------------------------//
