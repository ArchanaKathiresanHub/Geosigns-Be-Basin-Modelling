#include "TimeDependantLithology.h"

#include <algorithm>

//------------------------------------------------------------//

bool GeoPhysics::LithologyAgeOlderThan::operator ()( const LithologyAgePtr l1, const LithologyAgePtr l2 ) {
   return l1->startAge > l2->startAge;
}

//------------------------------------------------------------//

void GeoPhysics::LithologyAgeFindAge::setAge ( const double age ) {
   lithologyAge = age;
}

//------------------------------------------------------------//

bool GeoPhysics::LithologyAgeFindAge::operator ()( const LithologyAgePtr l1 ) const {
   return lithologyAge == l1->startAge;
}

//------------------------------------------------------------//

GeoPhysics::TimeDependantLithology::TimeDependantLithology () {

   currentLithology = 0;
   previousLithology = 0;
   currentLithologyStartAge = MaximumBasinAge;
   stratigraphyTableLithologyStartAge = MaximumBasinAge;
   isAllochthonous = false;

}

//------------------------------------------------------------//

GeoPhysics::TimeDependantLithology::~TimeDependantLithology () {

   size_t I;

   for ( I = 0; I < m_lithologies.size(); I++ ) {
      delete m_lithologies [ I ];
   }

   m_lithologies.clear ();

}

//------------------------------------------------------------//

void GeoPhysics::TimeDependantLithology::addStratigraphyTableLithology ( const double             age,
                                                                               CompoundLithology* newLithology ) {

   stratigraphyTableLithology = newLithology;
   stratigraphyTableLithologyStartAge = MaximumBasinAge;
   addLithology ( MaximumBasinAge, newLithology, PREFER_LAST_ON_LIST );

   // Set the current active lithology to that defined in the strat table.
   currentLithology = stratigraphyTableLithology;
   currentLithologyStartAge = stratigraphyTableLithologyStartAge;

   previousLithology = stratigraphyTableLithology;
}

//------------------------------------------------------------//

void GeoPhysics::TimeDependantLithology::addLithology ( const double                          age,
                                                              CompoundLithology*              newLithology,
                                                        const DuplicateLithologyAgePreference duplicatePreference  ) {

   CompoundLithology* addedLithology;


   // If the lithology is null, then there is nothing to do.
   if ( newLithology == 0 ) {
      addedLithology = stratigraphyTableLithology;
   } else {
      addedLithology = newLithology;
   }

   LithologyAgeOlderThan lessThan;
   LithologyAgeFindAge   findAge;

   findAge.setAge ( age );

   LithologyAgeVector::iterator result = find_if ( m_lithologies.begin (), m_lithologies.end (), findAge );

   if ( result == m_lithologies.end () || age == stratigraphyTableLithologyStartAge ) {

      LithologyAgePtr Litho = new LithologyAge;

      Litho->theLithology = addedLithology;
      Litho->startAge     = age;

      m_lithologies.push_back ( Litho );
      std::sort ( m_lithologies.begin (), m_lithologies.end (), lessThan );
   } else {

      if ( duplicatePreference == PREFER_FIRST_ON_LIST ) {
         //
         // Nothing to do here, since a lithology already exists with this age.
         //
      } else if ( duplicatePreference == PREFER_LAST_ON_LIST ) {
         //
         // Just replace the previously defined lithology with the new one
         //
         (*result)->theLithology = addedLithology;
      } else {
         double VES = 0.0;
         double maxVES = 0.0;
         CompoundProperty porosity1;
         double normalPermeability1;
         double tangentialPermeability1;
      
         CompoundProperty porosity2;
         double normalPermeability2;
         double tangentialPermeability2;

         (*result)->theLithology->getSurfacePorosity ( porosity1 );
         addedLithology->getSurfacePorosity ( porosity2 );

         (*result)->theLithology->calcBulkPermeabilityNP ( VES, maxVES, porosity1, normalPermeability1, tangentialPermeability1 );
         addedLithology->calcBulkPermeabilityNP ( VES, maxVES, porosity2, normalPermeability2, tangentialPermeability2 );

         if ( duplicatePreference == PREFER_LOWEST_PERMEABILITY ) {

            if ( normalPermeability2 < normalPermeability1 || 
                 (( normalPermeability2 == normalPermeability1 ) && ( tangentialPermeability2 < tangentialPermeability1 ))) {
               (*result)->theLithology = addedLithology;
            }

         } else {

            if ( normalPermeability2 > normalPermeability1 || 
                 (( normalPermeability2 == normalPermeability1 ) && ( tangentialPermeability2 > tangentialPermeability1 ))) {
               (*result)->theLithology = addedLithology;
            }

         }

      }

   }

}

//------------------------------------------------------------//

void GeoPhysics::TimeDependantLithology::setCurrentLithology ( const double age ) {

   size_t I;

   previousLithology = currentLithology;

   // Must be false here, since we do not know what the allochthonous lithology is!
   isAllochthonous = false;

   if ( age >= stratigraphyTableLithologyStartAge ) {
      currentLithology = stratigraphyTableLithology;
      currentLithologyStartAge = stratigraphyTableLithologyStartAge;
   } else {
      currentLithology = 0;

      // It is possible to keep an iterator here then we only need check the
      // lithologies from the iterator position to the end of the array
      // rather than from the start of the array. 
      for ( I = 0; I < m_lithologies.size (); I++ ) {

         if ( m_lithologies [ I ]->startAge >= age ) {
            currentLithology  = m_lithologies [ I ]->theLithology;
            currentLithologyStartAge = m_lithologies [ I ]->startAge;
         } else {
            assert ( currentLithology != 0 );
            break;
         }

      }

   }

  isAllochthonous = ( currentLithology != stratigraphyTableLithology and not currentLithology->isFault ());

}

//------------------------------------------------------------//

bool GeoPhysics::TimeDependantLithology::currentIsAllochthonous () const {
   return isAllochthonous;
}

//------------------------------------------------------------//

bool GeoPhysics::TimeDependantLithology::lithologyHasSwitched () const {
   return currentLithology != previousLithology;
}

//------------------------------------------------------------//

GeoPhysics::CompoundLithology* GeoPhysics::TimeDependantLithology::activeAt ( const double age ) const {

   size_t I;
   CompoundLithology* lithology = 0;

   if ( age >= stratigraphyTableLithologyStartAge ) {
      lithology = stratigraphyTableLithology;
   } else {

      for ( I = 0; I < m_lithologies.size (); I++ ) {

         if ( m_lithologies [ I ]->startAge >= age ) {
            lithology = m_lithologies [ I ]->theLithology;
         } else {
            assert ( lithology != 0 );
            break;
         }

      }

   }

   return lithology;
}

//------------------------------------------------------------//

void GeoPhysics::TimeDependantLithology::Print () const {

   size_t I;

   cout << "  GeoPhysics::TimeDependantLithology::Print ages " << currentLithologyStartAge << ":  ";

   for ( I = 0; I < m_lithologies.size (); I++ ){
      cout << m_lithologies [ I ]->startAge << "  ";
   } 

   cout << endl;
}

