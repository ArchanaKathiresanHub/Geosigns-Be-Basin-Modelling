#include "CompoundLithologyArray.h"

#include "array.h"
#include "NumericFunctions.h"

GeoPhysics::CompoundLithologyArray::CompoundLithologyArray () {
   m_duplicatePreference = GeoPhysics::PREFER_LAST_ON_LIST;
   m_interpolator = 0;
   m_lithologies = 0;
}

//------------------------------------------------------------//

GeoPhysics::CompoundLithologyArray::~CompoundLithologyArray () {

   if ( m_lithologies != 0 ) {
      unsigned int i;
      unsigned int j;

      for ( i = 0; i < length ( 0 ); ++i ) {

         for ( j = 0; j < length ( 1 ); ++j ) {

            if ( m_lithologies [ i ][ j ] == 0 ) {
               delete m_lithologies [ i ][ j ];
            }

         }

      }

      ibs::Array<TimeDependentLithology*>::delete2d ( m_lithologies );
   }

}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithologyArray::setDuplicateLithologyAgePreference ( const DuplicateLithologyAgePreference newPreference ) {
   m_duplicatePreference = newPreference;
}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithologyArray::allocate ( const DataAccess::Interface::Grid* grid ) {

   unsigned int i;
   unsigned int j;

   m_first [ 0 ] = grid->firstI ( true );
   m_first [ 1 ] = grid->firstJ ( true );

   m_last [ 0 ] = grid->lastI ( true );
   m_last [ 1 ] = grid->lastJ ( true );

   m_size [ 0 ] = m_last [ 0 ] - m_first [ 0 ] + 1;
   m_size [ 1 ] = m_last [ 1 ] - m_first [ 1 ] + 1;

   m_lithologies = ibs::Array<TimeDependentLithology*>::create2d ( m_size [ 0 ], m_size [ 1 ]);

   for ( i = 0; i < length ( 0 ); ++i ) {

      for ( j = 0; j < length ( 1 ); ++j ) {
         m_lithologies [ i ][ j ] = new TimeDependentLithology;
      }

   }

}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithologyArray::print ( const unsigned int i, const unsigned int j ) const {

  m_lithologies [ i - first ( 0 )][ j - first ( 1 )]->print ();

}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithologyArray::print () const {

   unsigned int i;
   unsigned int j;

   for ( i = first ( 0 ); i <= last ( 0 ); i++ ) {

      for ( j = first ( 1 ); j <= last ( 1 ); j++ ) {
         print ( i, j );
      }

   }

}

//------------------------------------------------------------//


GeoPhysics::CompoundLithology* GeoPhysics::CompoundLithologyArray::operator ()( const unsigned int subscriptI, const unsigned int subscriptJ ) const {
   return m_lithologies [ subscriptI - first ( 0 )][ subscriptJ - first ( 1 )]->currentActiveLithology ();
}

//------------------------------------------------------------//

GeoPhysics::CompoundLithology* GeoPhysics::CompoundLithologyArray::operator ()( const unsigned int subscriptI, const unsigned int subscriptJ, const double Age ) const {
   return m_lithologies [ subscriptI - first ( 0 )][ subscriptJ - first ( 1 )]->activeAt ( Age );
}

//------------------------------------------------------------//

bool GeoPhysics::CompoundLithologyArray::validIndex ( const unsigned int subscriptI,
                                                      const unsigned int subscriptJ ) const {

   bool iInRange = NumericFunctions::inRange <unsigned int> ( subscriptI, m_first [ 0 ], m_last [ 0 ]);
   bool jInRange = NumericFunctions::inRange <unsigned int> ( subscriptJ, m_first [ 1 ], m_last [ 1 ]);

   return ( iInRange and jInRange ) and ( operator ()( subscriptI, subscriptJ ) != 0 );
}

//------------------------------------------------------------//


void GeoPhysics::CompoundLithologyArray::addLithology ( const unsigned int       subscriptI,
                                                        const unsigned int       subscriptJ,
                                                        const double             Age,
                                                        CompoundLithology* newLithology ) {

   m_lithologies [ subscriptI - first ( 0 )][ subscriptJ - first ( 1 )]->addLithology ( Age, newLithology, m_duplicatePreference );

}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithologyArray::addStratigraphyTableLithology ( const unsigned int       subscriptI,
                                                                         const unsigned int       subscriptJ,
                                                                         CompoundLithology* newLithology ) {

  m_lithologies [ subscriptI - first ( 0 )][ subscriptJ - first ( 1 )]->addStratigraphyTableLithology ( newLithology );

}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithologyArray::fillWithLithology ( CompoundLithology* newLithology ) {

  unsigned int i;
  unsigned int j;

   for ( i = 0; i < length ( 0 ); ++i ) {

      for ( j = 0; j < length ( 1 ); ++j )
      {
            m_lithologies [ i ][ j ]->addStratigraphyTableLithology ( newLithology );
      }

   }

}

//------------------------------------------------------------//

void GeoPhysics::CompoundLithologyArray::setAllochthonousInterpolator ( AllochthonousLithologyInterpolator* newInterpolator ) {
  m_interpolator = newInterpolator;
}

//------------------------------------------------------------//

bool GeoPhysics::CompoundLithologyArray::isAllochthonousLithology ( const unsigned int subscriptI, const unsigned int subscriptJ ) const {
  return m_lithologies [ subscriptI - first ( 0 )][ subscriptJ - first ( 1 )]->currentIsAllochthonous ();
}

//------------------------------------------------------------//

bool GeoPhysics::CompoundLithologyArray::isAllochthonousAtAge ( const unsigned int i,
                                                                const unsigned int j,
                                                                const double       age ) const {
  return m_lithologies [ i - first ( 0 )][ j - first ( 1 )]->isAllochthonousAtAge ( age );
}

//------------------------------------------------------------//

bool GeoPhysics::CompoundLithologyArray::hasSwitched ( const unsigned int subscriptI, const unsigned int subscriptJ ) const {
  return m_lithologies [ subscriptI - first ( 0 )][ subscriptJ - first ( 1 )]->lithologyHasSwitched ();
}

//------------------------------------------------------------//

bool GeoPhysics::CompoundLithologyArray::setCurrentLithologies ( const double Age ) {

  unsigned int i;
  unsigned int j;
  bool switchHasOccurred = false;

  if ( m_interpolator != 0 ) {
    m_interpolator->setInterpolator ( Age );
  }

   for ( i = 0; i < length ( 0 ); ++i ) {
      for ( j = 0; j < length ( 1 ); ++j ) {
         if( m_lithologies [ i ][ j ]->currentActiveLithology () != 0 )
         {
            m_lithologies [ i ][ j ]->setCurrentLithology ( Age );
            switchHasOccurred = switchHasOccurred or m_lithologies [ i ][ j ]->lithologyHasSwitched ();
         }
     }

  }

  return switchHasOccurred;
}

//------------------------------------------------------------//
