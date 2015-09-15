#ifndef _FASTCAULDRON__NODAL_3D_INDEX_ARRAY__H_
#define _FASTCAULDRON__NODAL_3D_INDEX_ARRAY__H_

#include "Mesh3DIndex.h"

/// The positions of the nodes of a 3d cube element.
///
/// Indexing is from the closed interval [0,7].
class Nodal3DIndexArray {

public :

   /// Get the const reference to the index at the position.
   const Mesh3DIndex& operator ()( const int position ) const;

   /// Get the reference to the index at the position.
   Mesh3DIndex& operator ()( const int position );

   /// Get the I at the position.
   int getI ( const int position ) const;

   /// Get the J at the position.
   int getJ ( const int position ) const;

   /// Get the K at the position, either local or global.
   int getK ( const int  position ) const;

   /// Get the local-k at the position.
   int getLocalK ( const int position ) const;

   /// Get the global-k at the position.
   int getGlobalK ( const int position ) const;


   /// The first position in the array (0).
   int first () const;

   /// The last position in the array (7).
   int last () const;

private :

   Mesh3DIndex  m_nodalIndices [ 8 ];

};

//------------------------------------------------------------//
// Inline functions.
//------------------------------------------------------------//


inline const Mesh3DIndex& Nodal3DIndexArray::operator ()( const int position ) const {
   return m_nodalIndices [ position ];
}

inline Mesh3DIndex& Nodal3DIndexArray::operator ()( const int position ) {
   return m_nodalIndices [ position ];
}

inline int Nodal3DIndexArray::getI ( const int position ) const {
   return m_nodalIndices [ position ].getI ();
}

inline int Nodal3DIndexArray::getJ ( const int position ) const {
   return m_nodalIndices [ position ].getJ ();
}

inline int Nodal3DIndexArray::getK ( const int  position ) const {
   return m_nodalIndices [ position ].getK ();
}

inline int Nodal3DIndexArray::getLocalK ( const int position ) const {
   return m_nodalIndices [ position ].getK ();
}

inline int Nodal3DIndexArray::first () const {
   return 0;
}

inline int Nodal3DIndexArray::last () const {
   return 7;
}


#endif // _FASTCAULDRON__NODAL_3D_INDEX_ARRAY__H_
