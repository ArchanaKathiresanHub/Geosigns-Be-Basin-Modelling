#ifndef _FASTCAULDRON__MESH_INDEX__H_
#define _FASTCAULDRON__MESH_INDEX__H_

#include <string>


/// The position in any of the 3d data structures.
class Mesh3DIndex {

public :

   enum LocalityIndicator { LOCAL, GLOBAL };


   /// Constructor
   Mesh3DIndex ();

   /// Set the i, j and local-k positions.
   void set ( const int i,
              const int j,
              const int localK );

   /// Set the global-k position.
   void set ( const int globalK );

   /// Get the i-position.
   int getI () const;

   /// Get the -position.
   int getJ () const;

   /// Get the k-position (either local or global)
   int getK ( const LocalityIndicator indicator = LOCAL ) const;

   /// Get the local-k-position.
   int getLocalK () const;

   /// Get the global-k-position.
   int getGlobalK () const;


   /// Assignment operator.
   Mesh3DIndex& operator=( const Mesh3DIndex& index );

   /// return the string representation of the index.
   std::string image () const;

private :

   /// I index in plane (always a global related value)
   int m_i;

   /// J index in plane (always a global related value)
   int m_j;

   /// K index in vertical direction.
   ///
   /// The first position is the k relative to the layer k-index set.
   /// The second position is the k relative to the domain k-index set.
   int m_ks [ 2 ];

   // /// K index in vertical relative to the layer k-index set.
   // int m_localK;

   // /// K index in vertical relative to the domain k-index set.
   // int m_globalK;

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

inline int Mesh3DIndex::getK ( const LocalityIndicator indicator ) const {
   return m_ks [ indicator ];
   // return ( indicator == LOCAL ? getLocalK () : getGlobalK ());
}


inline int Mesh3DIndex::getLocalK () const {
   return m_ks [ LOCAL ];
   // return m_localK;
}

inline int Mesh3DIndex::getGlobalK () const {
   return m_ks [ GLOBAL ];
   // return m_globalK;
}

#endif // _FASTCAULDRON__MESH_INDEX__H_
