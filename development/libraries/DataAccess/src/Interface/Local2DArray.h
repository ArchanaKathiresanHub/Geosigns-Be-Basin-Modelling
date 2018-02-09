//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DATAACCESS__LOCAL_2D_ARRAY_H
#define DATAACCESS__LOCAL_2D_ARRAY_H

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
        template<typename T>
        class Local2DArray {

        public:

            /// Constructor
            Local2DArray();

            /// Destructor
            ~Local2DArray();

                       /// \brief Disallow copy construction.
            Local2DArray( const Local2DArray& array ) = delete;

            /// Assignment opperator
            Local2DArray& operator=( const Local2DArray& array);

            /// Allocate the array with the grid map.
            void reallocate( const DataAccess::Interface::Grid* grid,
                             const bool                         includeGhostNodes = true );

            /// The first index of the array in the dimension.
            unsigned int first( const unsigned int dim ) const;

            /// The last index of the array in the dimension.
            unsigned int last( const unsigned int dim ) const;

            /// The size of the array along the dimension.
            unsigned int length( const unsigned int dim ) const;


            /// Access the item at position i, j.
            const T& operator ()( const unsigned int i,
                                  const unsigned int j ) const;

            /// Access the item at position i, j.
            T& operator ()( const unsigned int i,
                            const unsigned int j );

            /// Fill the array with a particular value.
            void fill( const T& value );


        private:

            T** m_values;

            unsigned int m_first[2];
            unsigned int m_last[2];
            unsigned int m_size[2];


        };

    }
}

//------------------------------------------------------------//
//   Inline functions.
//------------------------------------------------------------//

template <typename T>
DataAccess::Interface::Local2DArray<T>::Local2DArray () {
   m_values = 0;
   m_first [ 0 ] = 0;
   m_first [ 1 ] = 0;

   m_last [ 0 ] = 0;
   m_last [ 1 ] = 0;

   m_size [ 0 ] = 0;
   m_size [ 1 ] = 0;
}

//------------------------------------------------------------//

template <typename T>
DataAccess::Interface::Local2DArray<T>::~Local2DArray () {

   if ( m_values != nullptr ) {
      ibs::Array<T>::delete2d ( m_values );
   }

}

//------------------------------------------------------------//

template <typename T>
void DataAccess::Interface::Local2DArray<T>::reallocate ( const DataAccess::Interface::Grid* grid,
                                                          const bool                         includeGhostNodes ) {

   if ( m_values != nullptr ) {
      ibs::Array<T>::delete2d ( m_values );
   }

   m_first [ 0 ] = grid->firstI ( includeGhostNodes );
   m_first [ 1 ] = grid->firstJ ( includeGhostNodes );

   m_last [ 0 ] = grid->lastI ( includeGhostNodes );
   m_last [ 1 ] = grid->lastJ ( includeGhostNodes );

   m_size [ 0 ] = m_last [ 0 ] - m_first [ 0 ] + 1;
   m_size [ 1 ] = m_last [ 1 ] - m_first [ 1 ] + 1;

   m_values = ibs::Array<T>::create2d ( m_size [ 0 ], m_size [ 1 ]);
}

//------------------------------------------------------------//

template <typename T>
inline const T& DataAccess::Interface::Local2DArray<T>::operator ()( const unsigned int i,
                                                                     const unsigned int j ) const {
   return m_values [ i - m_first [ 0 ]][ j - m_first [ 1 ]];
}

//------------------------------------------------------------//

template <typename T>
inline T& DataAccess::Interface::Local2DArray<T>::operator ()( const unsigned int i,
                                                               const unsigned int j ) {
   return m_values [ i - m_first [ 0 ]][ j - m_first [ 1 ]];
}

//------------------------------------------------------------//

template <typename T>
void DataAccess::Interface::Local2DArray<T>::fill ( const T& value ) {
   for ( unsigned int i = 0; i < m_size [ 0 ]; ++i ) {

      for ( unsigned int j = 0; j < m_size [ 1 ]; ++j ) {
         m_values [ i ][ j ] = value;
      }

   }

}

//------------------------------------------------------------//

template <typename T>
inline unsigned int DataAccess::Interface::Local2DArray<T>::first ( const unsigned int dim ) const {
   assert ( /* 0 <= dim and */ dim < 2 );
   return m_first [ dim ];
}

//------------------------------------------------------------//

template <typename T>
inline unsigned int DataAccess::Interface::Local2DArray<T>::last ( const unsigned int dim ) const {
   assert ( /* 0 <= dim and */ dim < 2 );
   return m_last [ dim ];
}

//------------------------------------------------------------//

template <typename T>
inline unsigned int DataAccess::Interface::Local2DArray<T>::length ( const unsigned int dim ) const {
   assert ( /* 0 <= dim and */ dim < 2 );
   return m_size [ dim ];
}

//------------------------------------------------------------//

template <typename T>
inline DataAccess::Interface::Local2DArray<T>& DataAccess::Interface::Local2DArray<T>::operator=( const DataAccess::Interface::Local2DArray<T>& array ) {
   if ( m_values != nullptr ) {
      ibs::Array<T>::delete2d ( m_values );
   }

   m_first[0] = array.m_first[0];
   m_first[1] = array.m_first[1];
   m_last[0]  = array.m_last[0];
   m_last[1]  = array.m_last[1];
   m_size[0]  = array.m_size[0];
   m_size[1]  = array.m_size[1];

   m_values = ibs::Array<T>::create2d ( m_size [ 0 ], m_size [ 1 ], array.m_values);

   return *this;
}

#endif // DATAACCESS__LOCAL_2D_ARRAY_H
