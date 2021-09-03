#ifndef _GridDescription_H_
#define _GridDescription_H_

#include <istream>

namespace database {
   class Record;
}

namespace DataAccess { namespace Interface { 
   class Grid;
} }

/// The model grid description.
///
/// This contains both the voxet-grid-description and the Cauldron map-description.
class GridDescription {

public :

   GridDescription (       database::Record* cauldronRecord,
                           database::Record* voxetRecord,
                     const DataAccess::Interface::Grid*  cauldronGrid );

   GridDescription ( const GridDescription& gridDescription );

   ~GridDescription();

   GridDescription(const GridDescription&& gridDescription) = delete;
   GridDescription const& operator=(GridDescription&& gridDescription) = delete;
   GridDescription& operator=(const GridDescription& gridDescription) = delete;

   /// \name Voxet data
   //@{

   /// The Voxet grid data is 3d data.

   /// Origin of the Cauldron grid.
   double getVoxetGridOrigin ( const unsigned int axes ) const;

   /// Vozet grid maxima.
   double getVoxetGridMaximum ( const unsigned int axes ) const;

   /// Voxet grid delta_{x,y,z}.
   double getVoxetGridDelta ( const unsigned int axes ) const;

   /// Voxet grid node count.
   int getVoxetNodeCount ( const unsigned int axes ) const;

   /// Returns the total number of nodes in the voxet grid.
   int getVoxetNodeCount () const;

   //@}

   /// \name Cauldron data
   //@{

   /// The Cauldron grid data is only 2d data.

   /// Origin of the Cauldron grid.
   double getCauldronGridOrigin ( const unsigned int axes,
                                 const bool         windowArea ) const;

   /// Cauldron grid maxima.
   double getCauldronGridMaximum ( const unsigned int axes,
                                  const bool         windowArea ) const;

   /// Cauldron grid delta_{x,y}.
   double getCauldronGridDelta ( const unsigned int axes ) const;

   /// Cauldron grid node count.
   int getCauldronNodeCount ( const unsigned int axes ) const;

   /// Return a const pointer to the cauldron grid map.
   const DataAccess::Interface::Grid* getCauldronGrid () const;

   //@}


private :

   database::Record* m_cauldronRecord;
   database::Record* m_voxetRecord;

   double m_voxetOrigin [ 3 ];
   double m_voxetMaxima [ 3 ];
   double m_voxetDelta [ 3 ];
   int   m_voxetNodeCount [ 3 ];

   double m_cauldronOrigin [ 2 ][ 2 ];
   double m_cauldronMaxima [ 2 ][ 2 ];
   double m_cauldronDelta [ 2 ];
   int   m_cauldronNodeCount [ 2 ];

   const DataAccess::Interface::Grid* m_cauldronGrid;
   

};

#endif // _GridDescription_H_
