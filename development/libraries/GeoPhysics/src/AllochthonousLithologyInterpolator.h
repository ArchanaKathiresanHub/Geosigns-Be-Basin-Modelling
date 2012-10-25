#ifndef _GEOPHYSICS__ALLOCHTHONOUS_LITHOLOGY_INTERPOLATOR_H_
#define _GEOPHYSICS__ALLOCHTHONOUS_LITHOLOGY_INTERPOLATOR_H_

#include <vector>

// Numerics
#include "Numerics.h"
#include "Point.h"
#include "IntervalInterpolator.h"

#include "CauldronGridDescription.h"

#include "CompoundLithology.h"


namespace GeoPhysics {

   /// Contains the lithology interpolators for a single formation.
   class AllochthonousLithologyInterpolator {

      /// \brief Used in sorting the distribution maps by age.
      class IntervalInterpolatorEarlierThan {

      public :

         bool operator ()( const Numerics::IntervalInterpolator* interval1, 
                           const Numerics::IntervalInterpolator* interval2 ) const;

      };

      /// \typedef IntervalInterpolatorSequence
      /// \brief The sequence of interval interpolators.
      typedef std::vector <Numerics::IntervalInterpolator*> IntervalInterpolatorSequence;


   public :


      typedef std::vector<double> MorphingTimeSteps;

      typedef IntervalInterpolatorSequence::const_iterator const_iterator;


      AllochthonousLithologyInterpolator ( GeoPhysics::CompoundLithology* initialLithology );

      void setGridDescription ( const CauldronGridDescription& newDescription );

      /// Insert an interpolator to the formation interpolator.
      void insert ( Numerics::IntervalInterpolator* newInterpolator );

      /// \brief Sort all interval-interpolators into decending order.
      ///
      /// Oldest first. When all the interpolators have been added to
      /// this class then this must be called.
      void freeze ( const int optimisationLevel );

      void reset ();

      /// Set which interval the subsequent evaluations will apply.
      void setInterpolator ( const double age );

      /// \brief Returns either the allochthonous-lithology or a null.
      ///
      /// The interpolator is evaluated, and if greater than, or equal to, zero, a pointer to 
      /// the allochthonous-lithology is returned, otherwise a null pointer is returned
      GeoPhysics::CompoundLithology* operator ()( const int    i,
                                                  const int    j,
                                                  const double age );
  
      /// \brief Return the first interval-interpolator in the sequence.
      const_iterator begin () const;

      /// \brief Return the end of the sequence of interval-interpolators.
      const_iterator end () const;

      const MorphingTimeSteps& getMorphingTimeSteps () const;

      static int maximumNumberOfMorphingTimeSteps ( const int optimisationLevel );

      static int minimumNumberOfMorphingTimeSteps ( const int optimisationLevel );

      static double morphingTimeStepSize          ( const int optimisationLevel );

   private :

      Numerics::Point getPoint ( const int    i,
                                 const int    j,
                                 const double age ) const;

      Numerics::IntervalInterpolator const*  currentInterpolator;
      IntervalInterpolatorSequence intervalInterpolators;

      CauldronGridDescription gridDescription;
      GeoPhysics::CompoundLithology* lithology;

      MorphingTimeSteps timestepsForMorphing;
      int currentAgePosition;

   };

}

#endif // _GEOPHYSICS__ALLOCHTHONOUS_LITHOLOGY_INTERPOLATOR_H_
