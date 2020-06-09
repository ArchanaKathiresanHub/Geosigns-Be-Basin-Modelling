#include "PropertiesToQuadPts.h"
#include "FiniteElementTypes.h"

namespace FiniteElementMethod
{
   PropertiesToQuadPts::~PropertiesToQuadPts()
   {
      // Intentionally unimplemented
   }


   PropertiesToQuadPts::PropertiesToQuadPts( const Numerics::AlignedDenseMatrix & basisMat )
      : m_numProps( 0 ),
        m_basisMat( basisMat ),
        m_cpuInfo( cpuInfo())
   {
      m_basisMatTranspose.resize ( m_basisMat.cols (), m_basisMat.rows ());
      Numerics::transpose ( m_basisMat, m_basisMatTranspose );
   }

   void PropertiesToQuadPts::addSingleProperty( const unsigned int propIdx,
                                                CauldronPropertyName propName,
                                                const ElementVector & propVal )
   {
      // Storing property name
      m_propNames[ propIdx ] = propName;
      // Filling matrix column with property values
      double * column = m_propOnDofsMat.getColumn( propIdx );
      std::copy( propVal.data(), propVal.data() + ElementVector::NumberOfEntries, column );
   }
}
