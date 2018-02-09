//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DATAACCESS_LOCAL_3D_ARRAY_H
#define DATAACCESS_LOCAL_3D_ARRAY_H

//std
#include <cassert>

//DataAccess
#include "Interface/Grid.h"

//utilities
#include "array.h"

namespace DataAccess
{
   namespace Interface
   {

      /// An generic array whose dimensions are those of the local grid, including the ghost nodes.
      ///
      /// There is no communication performed between the various processes to
      /// keep the array up-to-date.
      ///
      /// The k-dimension (3rd dimension) is the "depth" of the grid.
      template<typename T>
      class Local3DArray {

      public:

         /// Constructor
         Local3DArray();

         /// Constructor
         Local3DArray( const DataAccess::Interface::Grid* grid,
                       const unsigned int                 depth,
                       const bool                         includeGhostNodes = true );

         /// Destructor
         ~Local3DArray();

         /// \brief Disallow copy construction.
         Local3DArray( const Local3DArray& ) = delete;

         /// \brief Disallow assignment.
         Local3DArray& operator=( const Local3DArray& ) = delete;

         /// Allocate the array with the grid map.
         void reallocate( const DataAccess::Interface::Grid* grid,
                          const unsigned int                 depth,
                          const bool                         includeGhostNodes = true );

         /// The first index of the array in the dimension.
         unsigned int first( const unsigned int dim ) const;

         /// The last index of the array in the dimension.
         unsigned int last( const unsigned int dim ) const;

         /// The size of the array along the dimension.
         unsigned int length( const unsigned int dim ) const;


         /// Access the item at position i, j, k.
         const T& operator ()( const unsigned int i,
                               const unsigned int j,
                               const unsigned int k ) const;

         /// Access the item at position i, j, k.
         T& operator ()( const unsigned int i,
                         const unsigned int j,
                         const unsigned int k );

         /// Fill the array with a particular value.
         void fill( const T& value );


      private:

         T*** m_values;

         unsigned int m_first[3];
         unsigned int m_last[3];
         unsigned int m_size[3];


      };

   }

}

//------------------------------------------------------------//
//   Inline functions.
//------------------------------------------------------------//

template <typename T>
DataAccess::Interface::Local3DArray<T>::Local3DArray () {
   m_values = 0;

   m_first [ 0 ] = 0;
   m_first [ 1 ] = 0;
   m_first [ 2 ] = 0;

   m_last [ 0 ] = 0;
   m_last [ 1 ] = 0;
   m_last [ 2 ] = 0;

   m_size [ 0 ] = 0;
   m_size [ 1 ] = 0;
   m_size [ 2 ] = 0;
}

//------------------------------------------------------------//

template <typename T>
DataAccess::Interface::Local3DArray<T>::Local3DArray ( const DataAccess::Interface::Grid* grid,
                                                       const unsigned int                 depth,
                                                       const bool                         includeGhostNodes ) {
   m_values = 0;
   reallocate ( grid, depth, includeGhostNodes );
}

//------------------------------------------------------------//

template <typename T>
DataAccess::Interface::Local3DArray<T>::~Local3DArray () {

   if ( m_values != nullptr ) {
      ibs::Array<T>::delete3d ( m_values );
   }

}

//------------------------------------------------------------//

template <typename T>
void DataAccess::Interface::Local3DArray<T>::reallocate ( const DataAccess::Interface::Grid* grid,
                                                          const unsigned int                 depth,
                                                          const bool                         includeGhostNodes ) {

   if ( m_values != nullptr ) {
      ibs::Array<T>::delete3d ( m_values );
   }

   m_first [ 0 ] = grid->firstI ( includeGhostNodes );
   m_first [ 1 ] = grid->firstJ ( includeGhostNodes );
   m_first [ 2 ] = 0;

   m_last [ 0 ] = grid->lastI ( includeGhostNodes );
   m_last [ 1 ] = grid->lastJ ( includeGhostNodes );
   m_last [ 2 ] = depth - 1;

   m_size [ 0 ] = m_last [ 0 ] - m_first [ 0 ] + 1;
   m_size [ 1 ] = m_last [ 1 ] - m_first [ 1 ] + 1;
   m_size [ 2 ] = m_last [ 2 ] - m_first [ 2 ] + 1;

   m_values = ibs::Array<T>::create3d ( m_size [ 0 ], m_size [ 1 ], m_size [ 2 ]);
}

//------------------------------------------------------------//

template <typename T>
const T& DataAccess::Interface::Local3DArray<T>::operator ()( const unsigned int i,
                                                              const unsigned int j,
                                                              const unsigned int k ) const {

#if 0
   assert ( m_first [ 0 ] <= i and i <= m_last  [ 0 ]);
   assert ( m_first [ 1 ] <= j and j <= m_last  [ 1 ]);
   assert ( m_first [ 2 ] <= k and k <= m_last  [ 2 ]);
#endif

   return m_values [ i - m_first [ 0 ]][ j - m_first [ 1 ]][ k - m_first [ 2 ]];
}

//------------------------------------------------------------//

template <typename T>
T& DataAccess::Interface::Local3DArray<T>::operator ()( const unsigned int i,
                                                        const unsigned int j,
                                                        const unsigned int k ) {

#if 0
   assert ( m_first [ 0 ] <= i and i <= m_last  [ 0 ]);
   assert ( m_first [ 1 ] <= j and j <= m_last  [ 1 ]);
   assert ( m_first [ 2 ] <= k and k <= m_last  [ 2 ]);
#endif

   return m_values [ i - m_first [ 0 ]][ j - m_first [ 1 ]][ k - m_first [ 2 ]];
}

//------------------------------------------------------------//

template <typename T>
void DataAccess::Interface::Local3DArray<T>::fill ( const T& value ) {

   for ( unsigned int i = 0; i < m_size [ 0 ]; ++i ) {
      for ( unsigned int j = 0; j < m_size [ 1 ]; ++j ) {
         for ( unsigned int k = 0; k < m_size [ 2 ]; ++k ) {
            m_values [ i ][ j ][ k ] = value;
         }
      }
   }

}

//------------------------------------------------------------//

template <typename T>
unsigned int DataAccess::Interface::Local3DArray<T>::first ( const unsigned int dim ) const {
   assert ( /* 0 <= dim and */ dim < 3 );
   return m_first [ dim ];
}

//------------------------------------------------------------//

template <typename T>
unsigned int DataAccess::Interface::Local3DArray<T>::last ( const unsigned int dim ) const {
   assert ( /* 0 <= dim and */ dim < 3 );
   return m_last [ dim ];
}

//------------------------------------------------------------//

template <typename T>
unsigned int DataAccess::Interface::Local3DArray<T>::length ( const unsigned int dim ) const {
   assert ( /* 0 <= dim and */ dim < 3 );
   return m_size [ dim ];
}


#endif
