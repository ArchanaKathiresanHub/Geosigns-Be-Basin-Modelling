#ifndef _GridDescription_H_
#define _GridDescription_H_

#include <istream>

#include "database.h"
#include "Grid.h"


using namespace DataAccess;
using namespace Interface;

/// The model grid description.
///
/// This contains both the voxet-grid-description and the Cauldron map-description.
class GridDescription {

public :

   GridDescription (       database::Record* cauldronRecord,
                           database::Record* voxetRecord,
                     const Interface::Grid*  cauldronGrid );

   GridDescription ( const GridDescription& gridDescription );

   /// \name Voxet data
   //@{

   /// The Voxet grid data is 3d data.

   /// Origin of the Cauldron grid.
   float getVoxetGridOrigin ( const unsigned int axes ) const;

   /// Vozet grid maxima.
   float getVoxetGridMaximum ( const unsigned int axes ) const;

   /// Voxet grid delta_{x,y,z}.
   float getVoxetGridDelta ( const unsigned int axes ) const;

   /// Voxet grid node count.
   int getVoxetNodeCount ( const unsigned int axes ) const;

   /// Returns the total number of nodes in the voxet grid.
   int getVoxetNodeCount () const;

   //@}

   /// \name Cauldron data
   //@{

   /// The Cauldron grid data is only 2d data.

   /// Origin of the Cauldron grid.
   float getCauldronGridOrigin ( const unsigned int axes,
                                 const bool         windowArea ) const;

   /// Cauldron grid maxima.
   float getCauldronGridMaximum ( const unsigned int axes,
                                  const bool         windowArea ) const;

   /// Cauldron grid delta_{x,y}.
   float getCauldronGridDelta ( const unsigned int axes ) const;

   /// Cauldron grid node count.
   int getCauldronNodeCount ( const unsigned int axes ) const;

   /// Return a const pointer to the cauldron grid map.
   const Interface::Grid* getCauldronGrid () const;

   //@}


private :

   database::Record* m_cauldronRecord;
   database::Record* m_voxetRecord;

   float m_voxetOrigin [ 3 ];
   float m_voxetMaxima [ 3 ];
   float m_voxetDelta [ 3 ];
   int   m_voxetNodeCount [ 3 ];

   float m_cauldronOrigin [ 2 ][ 2 ];
   float m_cauldronMaxima [ 2 ][ 2 ];
   float m_cauldronDelta [ 2 ];
   int   m_cauldronNodeCount [ 2 ];

   const Interface::Grid* m_cauldronGrid;
   

};

#endif // _GridDescription_H_
