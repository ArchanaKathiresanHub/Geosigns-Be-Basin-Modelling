#ifndef _FASTCAULDRON__MESH_INDEX__H_
#define _FASTCAULDRON__MESH_INDEX__H_

#include <string>


/// The position in any of the 3d data structures.
class Mesh3DIndex {

public :

   /// Constructor
   Mesh3DIndex ();

   /// Set the i, j and local-k positions.
   void set ( const int i,
              const int j,
              const int localK );

   /// Get the i-position.
   int getI () const;

   /// Get the -position.
   int getJ () const;

   /// Get the k-position (either local or global)
   int getK () const;

   /// Assignment operator.
   Mesh3DIndex& operator=( const Mesh3DIndex& index );

   /// return the string representation of the index.
   std::string image () const;

private :

   /// I index in plane (always a global related value)
   int m_i;

   /// J index in plane (always a global related value)
   int m_j;

   /// K index in vertical direction for the layer.
   int m_k;

};

//------------------------------------------------------------//
// Inline functions.
//------------------------------------------------------------//

inline int Mesh3DIndex::getI () const {
   return m_i;
}

inline int Mesh3DIndex::getJ () const {
   return m_j;
}

inline int Mesh3DIndex::getK () const {
   return m_k;
}

#endif // _FASTCAULDRON__MESH_INDEX__H_
