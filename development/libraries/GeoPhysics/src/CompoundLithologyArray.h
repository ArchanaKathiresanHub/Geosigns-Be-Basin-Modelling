#ifndef _GEOPHYSICS__COMPOUND_LITHOLOGY_ARRAY_H_
#define _GEOPHYSICS__COMPOUND_LITHOLOGY_ARRAY_H_

#include "Interface/Grid.h"

#include "AllochthonousLithologyInterpolator.h"
#include "CompoundLithology.h"
#include "TimeDependantLithology.h"

namespace GeoPhysics {

   /// A two-dimensional array of TimeDependantLithology's.
   /// The array includes ghost-nodes.
   class CompoundLithologyArray {

   public :

      CompoundLithologyArray ();
      ~CompoundLithologyArray ();

      void allocate ( const DataAccess::Interface::Grid* grid );

      /// Returns the current active lithology (this must have been set beforehand)
      CompoundLithology*  operator ()( const unsigned int subscriptI, const unsigned int subscriptJ ) const;

      /// Returns the lithology at the desired age.
      CompoundLithology*  operator ()( const unsigned int subscriptI, const unsigned int subscriptJ, const double Age ) const;


      void addLithology ( const unsigned int       subscriptI,
                          const unsigned int       subscriptJ,
                          const double             Age,
                                CompoundLithology* newLithology );

      void addStratigraphyTableLithology ( const unsigned int       subscriptI,
                                           const unsigned int       subscriptJ,
                                           const double             Age,
                                                 CompoundLithology* newLithology );

      /// Fills array with same lithology.
      void fillWithLithology ( const double             Age,
                                     CompoundLithology* newLithology );

      /// Set the 'cached' lithology, this saves searching for the required lithology each time it's accessed.
      bool setCurrentLithologies ( const double Age );

      /// Set the interpolator for the allochthonous modelling.
      void setAllochthonousInterpolator ( AllochthonousLithologyInterpolator* newInterpolator );

      /// If 2 or more lithologies are assigned with the same age, which one of them takes precedence.
      /// This is determined by the DuplicateLithologyAgePreference.
      void setDuplicateLithologyAgePreference ( const DuplicateLithologyAgePreference newPreference );

      bool isAllochthonousLithology ( const unsigned int subscriptI,
                                      const unsigned int subscriptJ ) const;

      /// Returns whether the lithology at any age is part of the allochthonous modelling.
      bool isAllochthonousAtAge ( const unsigned int i,
                                  const unsigned int j,
                                  const double       age ) const;

      bool hasSwitched ( const unsigned int subscriptI,
                         const unsigned int subscriptJ ) const;

      bool validIndex ( const unsigned int subscriptI,
                        const unsigned int subscriptJ ) const;

      bool hasAllochthonousLithologyInterpolator () const;

      void print ( const unsigned int i,
                   const unsigned int j ) const;

      void print () const;

      unsigned int first  ( const unsigned int dim ) const;

      unsigned int last   ( const unsigned int dim ) const;

      unsigned int length ( const unsigned int dim ) const;

   private :

      TimeDependantLithology*** m_lithologies;

      unsigned int m_first [ 2 ];
      unsigned int m_last  [ 2 ];
      unsigned int m_size  [ 2 ];

      AllochthonousLithologyInterpolator* interpolator;
      DuplicateLithologyAgePreference duplicatePreference;

   };

}

inline unsigned int GeoPhysics::CompoundLithologyArray::first ( const unsigned int dim ) const {
   assert ( /* 0 <= dim and */ dim < 2 );
   return m_first [ dim ];
}

inline unsigned int GeoPhysics::CompoundLithologyArray::last ( const unsigned int dim ) const {
   assert ( /* 0 <= dim and */ dim < 2 );
   return m_last [ dim ];
}

inline unsigned int GeoPhysics::CompoundLithologyArray::length ( const unsigned int dim ) const {
   assert ( /* 0 <= dim and */ dim < 2 );
   return m_size [ dim ];
}


#endif // _GEOPHYSICS__COMPOUND_LITHOLOGY_ARRAY_H_
