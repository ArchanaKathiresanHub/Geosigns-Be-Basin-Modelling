#include "PropertiesToQuadPts.h"
#include "FiniteElementTypes.h"
#include <algorithm>
#include <assert.h>

namespace FiniteElementMethod
{
   PropertiesToQuadPts::~PropertiesToQuadPts()
   {}
  

   PropertiesToQuadPts::PropertiesToQuadPts( const Numerics::AlignedDenseMatrix & basisMat )
      : m_numProps( 0 ),
        m_basisMat( basisMat )
   {}


   const double * PropertiesToQuadPts::getProperty( const unsigned int idx ) const
   {
      assert( idx < m_numProps );
      return m_propOnQuadMat.getColumn( idx );
   }


   const double * PropertiesToQuadPts::getProperty( Property propName ) const
   {
      const unsigned int idx = static_cast<unsigned int>( std::find( m_propNames.begin(),
                                                                     m_propNames.end(), propName )
                                                          - m_propNames.begin() );
      return getProperty( idx );
   }


   void PropertiesToQuadPts::addSingleProperty( const unsigned int propIdx,
                                                Property propName,
                                                const ElementVector & propVal )
   {
      // Storing property name
      m_propNames[ propIdx ] = propName;
      // Filling matrix column with property values
      double * column = m_propOnDofsMat.getColumn( propIdx );
      std::copy( propVal.data(), propVal.data() + ElementVector::NumberOfEntries, column );
   }
}
