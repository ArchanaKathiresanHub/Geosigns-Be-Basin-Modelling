#ifndef _VOXET_PROPERTY_GRID_H_
#define _VOXET_PROPERTY_GRID_H_


#include "GridDescription.h"

/// A grid of data-values for the voxet-grid.
class VoxetPropertyGrid {

public :

   VoxetPropertyGrid ( const GridDescription& grid );

   ~VoxetPropertyGrid ();

   /// Value at the specified position.
   float operator ()( const unsigned int i,
                      const unsigned int j,
                      const unsigned int k ) const;

   /// Value at the specified position.
   float& operator ()( const unsigned int i,
                       const unsigned int j,
                       const unsigned int k );

   /// Get all data in a simple contiguous array.
   float* getOneDData () const;

   const GridDescription& getGridDescription () const;

private :

   const GridDescription& m_grid;

   /// The property-values, they are stored in "reverse", order. (k, j, i)
   float*** m_propertyValues;

};

// Some inline functions.

inline float VoxetPropertyGrid::operator ()( const unsigned int i,
                                             const unsigned int j,
                                             const unsigned int k ) const {
   return m_propertyValues [ k ][ j ][ i ];
}

inline float& VoxetPropertyGrid::operator ()( const unsigned int i,
                                              const unsigned int j,
                                              const unsigned int k ) {
   return m_propertyValues [ k ][ j ][ i ];
}


#endif // _VOXET_PROPERTY_GRID_H_
