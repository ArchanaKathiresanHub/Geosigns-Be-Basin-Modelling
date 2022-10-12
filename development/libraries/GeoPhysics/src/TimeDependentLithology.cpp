#include "TimeDependentLithology.h"

#include <algorithm>
using namespace std;

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

GeoPhysics::TimeDependentLithology::TimeDependentLithology () {

   m_currentLithology = 0;
   m_previousLithology = 0;
   m_currentLithologyStartAge = MaximumBasinAge;
   m_stratigraphyTableLithologyStartAge = MaximumBasinAge;
   m_isAllochthonous = false;

}

//------------------------------------------------------------//

GeoPhysics::TimeDependentLithology::~TimeDependentLithology () {

   size_t I;

   for ( I = 0; I < m_lithologies.size(); I++ ) {
      delete m_lithologies [ I ];
   }

   m_lithologies.clear ();

}

//------------------------------------------------------------//

void GeoPhysics::TimeDependentLithology::addStratigraphyTableLithology ( CompoundLithology* newLithology ) {

   m_stratigraphyTableLithology = newLithology;
   m_stratigraphyTableLithologyStartAge = MaximumBasinAge;
   addLithology ( MaximumBasinAge, newLithology, PREFER_LAST_ON_LIST );

   // Set the current active lithology to that defined in the strat table.
   m_currentLithology = m_stratigraphyTableLithology;
   m_currentLithologyStartAge = m_stratigraphyTableLithologyStartAge;

   m_previousLithology = m_stratigraphyTableLithology;
}

//------------------------------------------------------------//

void GeoPhysics::TimeDependentLithology::addLithology ( const double                          age,
                                                              CompoundLithology*              newLithology,
                                                        const DuplicateLithologyAgePreference duplicatePreference  ) {

   CompoundLithology* addedLithology;


   // If the lithology is null, then there is nothing to do.
   if ( newLithology == 0 ) {
      addedLithology = m_stratigraphyTableLithology;
   } else {
      addedLithology = newLithology;
   }

   LithologyAgeOlderThan lessThan;
   LithologyAgeFindAge   findAge;

   findAge.setAge ( age );

   LithologyAgeVector::iterator result = find_if ( m_lithologies.begin (), m_lithologies.end (), findAge );

   if ( result == m_lithologies.end () || age == m_stratigraphyTableLithologyStartAge ) {

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

void GeoPhysics::TimeDependentLithology::setCurrentLithology ( const double age ) {

   size_t I;

   m_previousLithology = m_currentLithology;

   // Must be false here, since we do not know what the allochthonous lithology is!
   m_isAllochthonous = false;

   if ( age >= m_stratigraphyTableLithologyStartAge ) {
      m_currentLithology = m_stratigraphyTableLithology;
      m_currentLithologyStartAge = m_stratigraphyTableLithologyStartAge;
   } else {
      m_currentLithology = 0;

      // It is possible to keep an iterator here then we only need check the
      // lithologies from the iterator position to the end of the array
      // rather than from the start of the array. 
      for ( I = 0; I < m_lithologies.size (); I++ ) {

         if ( m_lithologies [ I ]->startAge >= age ) {
            m_currentLithology  = m_lithologies [ I ]->theLithology;
            m_currentLithologyStartAge = m_lithologies [ I ]->startAge;
         } else {
            assert ( m_currentLithology != 0 );
            break;
         }

      }

   }

   m_isAllochthonous = ( m_currentLithology != m_stratigraphyTableLithology and not m_currentLithology->isFault ());

}

//------------------------------------------------------------//

bool GeoPhysics::TimeDependentLithology::currentIsAllochthonous () const {
   return m_isAllochthonous;
}

//------------------------------------------------------------//

bool GeoPhysics::TimeDependentLithology::isAllochthonousAtAge ( const double age ) const {

   size_t i;

   bool isAllochthonous;

   if ( age >= m_stratigraphyTableLithologyStartAge ) {
      isAllochthonous = false;
   } else {
      CompoundLithology* lithology = 0;

      // It is possible to keep an iterator here then we only need check the
      // lithologies from the iterator position to the end of the array
      // rather than from the start of the array. 
      for ( i = 0; i < m_lithologies.size (); ++i ) {

         if ( m_lithologies [ i ]->startAge >= age ) {
            lithology = m_lithologies [ i ]->theLithology;
         } else {
            // As soon as we go past the the age we require then exit the loop
            break;
         }

      }

      isAllochthonous = ( lithology != 0 and lithology != m_stratigraphyTableLithology and not lithology->isFault ());
   }

   return isAllochthonous;
}

//------------------------------------------------------------//

bool GeoPhysics::TimeDependentLithology::lithologyHasSwitched () const {
   return m_currentLithology != m_previousLithology;
}

//------------------------------------------------------------//

GeoPhysics::CompoundLithology* GeoPhysics::TimeDependentLithology::activeAt ( const double age ) const {

   size_t I;
   CompoundLithology* lithology = 0;

   if ( age >= m_stratigraphyTableLithologyStartAge ) {
      lithology = m_stratigraphyTableLithology;
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

void GeoPhysics::TimeDependentLithology::print () const {

   size_t I;

   cout << "  GeoPhysics::TimeDependentLithology::Print ages " << m_currentLithologyStartAge << ":  ";

   for ( I = 0; I < m_lithologies.size (); I++ ){
      cout << m_lithologies [ I ]->startAge << "  ";
   } 

   cout << endl;
}

