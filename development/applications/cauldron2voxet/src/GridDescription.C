#include "GridDescription.h"
#include "voxetschemafuncs.h"
#include "cauldronschemafuncs.h"


GridDescription::GridDescription (       database::Record* cauldronRecord,
                                         database::Record* voxetRecord,
                                   const Interface::Grid*  cauldronGrid ) : 
   m_cauldronRecord ( cauldronRecord ),
   m_voxetRecord ( voxetRecord ) {

   m_cauldronGrid = cauldronGrid;

   m_voxetNodeCount [ 0 ] = database::getNumberOfVoxetNodesX ( m_voxetRecord );
   m_voxetNodeCount [ 1 ] = database::getNumberOfVoxetNodesY ( m_voxetRecord );
   m_voxetNodeCount [ 2 ] = database::getNumberOfVoxetNodesZ ( m_voxetRecord );

   m_voxetOrigin [ 0 ] = database::getVoxetOriginX ( m_voxetRecord );
   m_voxetOrigin [ 1 ] = database::getVoxetOriginY ( m_voxetRecord );
   m_voxetOrigin [ 2 ] = database::getVoxetOriginZ ( m_voxetRecord );

   m_voxetDelta [ 0 ] = database::getVoxetDeltaX ( m_voxetRecord );
   m_voxetDelta [ 1 ] = database::getVoxetDeltaY ( m_voxetRecord );
   m_voxetDelta [ 2 ] = database::getVoxetDeltaZ ( m_voxetRecord );

   m_voxetMaxima [ 0 ] = m_voxetOrigin [ 0 ] + ( m_voxetNodeCount [ 0 ] - 1 ) * m_voxetDelta [ 0 ];
   m_voxetMaxima [ 1 ] = m_voxetOrigin [ 1 ] + ( m_voxetNodeCount [ 1 ] - 1 ) * m_voxetDelta [ 1 ];
   m_voxetMaxima [ 2 ] = m_voxetOrigin [ 2 ] + ( m_voxetNodeCount [ 2 ] - 1 ) * m_voxetDelta [ 2 ];

   m_cauldronOrigin    [ 0 ][ 0 ] = m_cauldronGrid->minI ();
   m_cauldronOrigin    [ 1 ][ 0 ] = m_cauldronGrid->minJ ();

   m_cauldronMaxima    [ 0 ][ 0 ] = m_cauldronGrid->maxI ();
   m_cauldronMaxima    [ 1 ][ 0 ] = m_cauldronGrid->maxJ ();

   m_cauldronOrigin    [ 0 ][ 1 ] = database::getXCoord ( m_cauldronRecord );
   m_cauldronOrigin    [ 1 ][ 1 ] = database::getYCoord ( m_cauldronRecord );

   m_cauldronMaxima    [ 0 ][ 1 ] = database::getXCoord ( m_cauldronRecord ) + database::getNumberX ( m_cauldronRecord ) * database::getDeltaX ( m_cauldronRecord );
   m_cauldronMaxima    [ 1 ][ 1 ] = database::getYCoord ( m_cauldronRecord ) + database::getNumberY ( m_cauldronRecord ) * database::getDeltaY ( m_cauldronRecord );

   m_cauldronDelta     [ 0 ] = m_cauldronGrid->deltaI ();
   m_cauldronDelta     [ 1 ] = m_cauldronGrid->deltaJ ();

   m_cauldronNodeCount [ 0 ] = m_cauldronGrid->numI ();
   m_cauldronNodeCount [ 1 ] = m_cauldronGrid->numJ ();

}

GridDescription::GridDescription ( const GridDescription& gridDescription ) {

   unsigned int i;

   for ( i = 0; i < 3; ++i ) {
      m_voxetOrigin [ i ] = gridDescription.getVoxetGridOrigin ( i );
      m_voxetMaxima [ i ] = gridDescription.getVoxetGridMaximum ( i );
      m_voxetDelta [ i ] = gridDescription.getVoxetGridDelta ( i );
      m_voxetNodeCount [ i ] = gridDescription.getVoxetNodeCount ( i );
   }

   for ( i = 0; i < 2; ++i ) {
      m_cauldronOrigin [ i ][ 0 ] = gridDescription.getCauldronGridOrigin ( i, true );
      m_cauldronMaxima [ i ][ 0 ] = gridDescription.getCauldronGridMaximum ( i, true );
      m_cauldronOrigin [ i ][ 1 ] = gridDescription.getCauldronGridOrigin ( i, false );
      m_cauldronMaxima [ i ][ 1 ] = gridDescription.getCauldronGridMaximum ( i, false );
      m_cauldronDelta [ i ] = gridDescription.getCauldronGridDelta ( i );
      m_cauldronNodeCount [ i ] = gridDescription.getCauldronNodeCount ( i );
   }

   m_cauldronGrid = gridDescription.getCauldronGrid ();

}


float GridDescription::getVoxetGridOrigin ( const unsigned int axes ) const {
   return m_voxetOrigin [ axes ];
}

float GridDescription::getVoxetGridMaximum ( const unsigned int axes ) const {
   return m_voxetMaxima [ axes ];
}

float GridDescription::getVoxetGridDelta ( const unsigned int axes ) const {
   return m_voxetDelta [ axes ];
}

int GridDescription::getVoxetNodeCount ( const unsigned int axes ) const {
   return m_voxetNodeCount [ axes ];
}

int GridDescription::getVoxetNodeCount () const {
   return m_voxetNodeCount [ 0 ] * m_voxetNodeCount [ 1 ] * m_voxetNodeCount [ 2 ];
}

float GridDescription::getCauldronGridOrigin ( const unsigned int axes,
                                               const bool         windowArea ) const {
   return m_cauldronOrigin [ axes ][( windowArea ? 0 : 1 )];
}

float GridDescription::getCauldronGridMaximum ( const unsigned int axes,
                                                const bool         windowArea ) const {
   return m_cauldronMaxima [ axes ][( windowArea ? 0 : 1 )];
}

float GridDescription::getCauldronGridDelta ( const unsigned int axes ) const {
   return m_cauldronDelta [ axes ];
}

int GridDescription::getCauldronNodeCount ( const unsigned int axes ) const {
   return m_cauldronNodeCount [ axes ];
}

const Interface::Grid* GridDescription::getCauldronGrid () const {
   return m_cauldronGrid;
}
