#include "Mesh3DIndex.h"

#include <sstream>


Mesh3DIndex::Mesh3DIndex () {
   set ( -1, -1, -1 );
   set ( -1 );
}

void Mesh3DIndex::set ( const int i,
                        const int j,
                        const int localK ) {
   m_i = i;
   m_j = j;
   m_ks [ LOCAL ] = localK;
   // m_localK = localK;
}

void Mesh3DIndex::set ( const int globalK ) {
   m_ks [ GLOBAL ] = globalK;
   // m_globalK = globalK;
}


Mesh3DIndex& Mesh3DIndex::operator=( const Mesh3DIndex& index ) {

   m_i = index.m_i;
   m_j = index.m_j;
   m_ks [ 0 ] = index.m_ks [ 0 ];
   m_ks [ 1 ] = index.m_ks [ 1 ];

   return *this;
}

std::string Mesh3DIndex::image () const {

   std::stringstream buffer;

   buffer << "{ " 
          << m_i << ", "
          << m_j << ", "
          << m_ks [ LOCAL ] << ", "
          << m_ks [ GLOBAL ] << " }";
          // << m_localK << ", "
          // << m_globalK << " }";

   return buffer.str ();
}
