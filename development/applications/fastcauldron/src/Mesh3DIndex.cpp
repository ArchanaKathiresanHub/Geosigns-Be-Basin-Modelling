#include "Mesh3DIndex.h"

#include <sstream>


Mesh3DIndex::Mesh3DIndex () {
   set ( -1, -1, -1 );
}

void Mesh3DIndex::set ( const int i,
                        const int j,
                        const int localK ) {
   m_i = i;
   m_j = j;
   m_k = localK;
}

Mesh3DIndex& Mesh3DIndex::operator=( const Mesh3DIndex& index ) {

   m_i = index.m_i;
   m_j = index.m_j;
   m_k = index.m_k;

   return *this;
}

std::string Mesh3DIndex::image () const {

   std::stringstream buffer;

   buffer << "{ " 
          << m_i << ", "
          << m_j << ", "
          << m_k << " } ";

   return buffer.str ();
}
