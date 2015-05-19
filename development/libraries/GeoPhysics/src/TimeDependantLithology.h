#ifndef _GEOPHYSICS__TIME_DEPENDANT_LITHOLOGY_H_
#define _GEOPHYSICS__TIME_DEPENDANT_LITHOLOGY_H_

#include <vector>

#include "CompoundLithology.h"

namespace GeoPhysics {

   /// \var MaximumBasinAge
   /// No age will be (should be!!) greater that this.
   const double MaximumBasinAge = 5000.0; // Ma (Approximate age of the Earth)


   /// Simple struct containing a Lithology-Age pair.
   struct LithologyAge {
      CompoundLithology* theLithology;
      double             startAge;
   };

   typedef LithologyAge* LithologyAgePtr;


   /// Functor class enabling sorting of LithologyAge-pair objects w.r.t age.
   class LithologyAgeOlderThan {

   public :
      bool operator ()( const LithologyAgePtr L1, const LithologyAgePtr L2 );
   };


   /// Functor class enabling the finding of a LithologyAge-pair easily (by age)
   class LithologyAgeFindAge {

   public :

      void setAge ( const double age );

      bool operator ()( const LithologyAgePtr L1 ) const;

   private :

      double lithologyAge;

   };

   /// What to do if duplicate ages are added to the time dependant lithology is dictated by these values.
   enum DuplicateLithologyAgePreference { PREFER_FIRST_ON_LIST,
                                          PREFER_LAST_ON_LIST,
                                          PREFER_LOWEST_PERMEABILITY,
                                          PREFER_HIGHEST_PREMEABILITY };

   /// A sequence of lithologies, in time order.
   /// The lithology is active forwards in time, from the age defined.
   /// 
   /// 
   ///  |                 |
   ///  | L1 --->  ...    | L2 --->
   ///  |                 |
   ///  +--------- ... ---+-------...
   ///  age1              age2
   ///
   ///
   /// If a lithology is added with the same start time as one that already exists then 
   /// the behaviour depends on the paramater 'DuplicateLithologyAgePreference'.
   ///
   class TimeDependantLithology {

      typedef std::vector<LithologyAgePtr> LithologyAgeVector;

   public :

      TimeDependantLithology ();
      ~TimeDependantLithology ();

      /// Add the lithology that is defined in the stratigraphy table to the sequence.
      void addStratigraphyTableLithology ( const double             age,
                                                 CompoundLithology* newLithology );

      /// Add a lithology change to the sequence.
      ///
      /// If the new-lithology is null then the stratigraphy-table lithology will be added at this age.
      void addLithology ( const double                          age,
                                CompoundLithology*              newLithology,
                          const DuplicateLithologyAgePreference duplicatePreference );

      /// Set the active lithology to that defined in the lithology sequence.
      /// This saves searching the list of lithologies looking for the one
      /// that is closest but older that the parameter age.
      void setCurrentLithology ( const double age );

      /// Returns whether the current lithology is part of the allochthonous modelling.
      bool currentIsAllochthonous () const;

      /// Returns whether the lithology at any age is part of the allochthonous modelling.
      bool isAllochthonousAtAge ( const double age ) const;

      /// Returns whether the lithology has changed in the last switching request.
      ///
      /// If the lithology has switched to the same lithology this will not be 
      /// indicated as a lithology-switch.
      bool lithologyHasSwitched () const;

      CompoundLithology* currentActiveLithology () const;

      /// Returns the lithology that is active at the age.
      CompoundLithology* activeAt ( const double age ) const;

      void Print () const;

   private :

      /// Change to LithologyAge class!!!!!
      CompoundLithology* stratigraphyTableLithology;
      double             stratigraphyTableLithologyStartAge;

      CompoundLithology* previousLithology;
      CompoundLithology* currentLithology;
      double             currentLithologyStartAge;

      LithologyAgeVector m_lithologies;

      bool               isAllochthonous;

   };

}

//------------------------------------------------------------//
//  Inline functions.
//------------------------------------------------------------//

inline GeoPhysics::CompoundLithology* GeoPhysics::TimeDependantLithology::currentActiveLithology () const {
  return currentLithology;
}

#endif // _GEOPHYSICS__TIME_DEPENDANT_LITHOLOGY_H_
