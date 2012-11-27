#include <iostream>
#include <arpa/inet.h>
#include <cassert>

#include "petscmatrix.h"
#include "petscio.h"

namespace hpc
{

PetscMatrix
  :: PetscMatrix(SizeType numberOfRows, SizeType numberOfColumns)
  : m_rows(numberOfRows)
  , m_columns(numberOfColumns)
  , m_rowOffsets()
  , m_columnIndices()				
  , m_nonZeros()
{
}  

void
PetscMatrix
  :: insert( SizeType rowIndex, SizeType columnIndex, ValueType value)
{
  assert( rowIndex < m_rows );
  assert( columnIndex < m_columns );

  // ensure rowOffsets is large enough
  m_rowOffsets.resize( 
      std::max<SizeType>( m_rowOffsets.size(), rowIndex + 2 ),
      m_rowOffsets.empty() ? 0u : m_rowOffsets.back()
   );

  // determine the row in which to insert the new element
  SizeType rowBegin = m_rowOffsets[ rowIndex ];
  SizeType rowEnd = m_rowOffsets[ rowIndex + 1];

  // determine where in the row to insert the element
  SizeType insertPos = std::lower_bound( m_columnIndices.begin() + rowBegin,
	m_columnIndices.begin() + rowEnd,
	columnIndex
      ) - m_columnIndices.begin() ;

  if (insertPos < m_columnIndices.size() && m_columnIndices[insertPos] == columnIndex)
  { // then this insert replaces a value
    m_nonZeros[insertPos] = value;
    return;
  }
  

  // add the element
  m_columnIndices.insert( m_columnIndices.begin() + insertPos, columnIndex );
  m_nonZeros.insert( m_nonZeros.begin() + insertPos, value);
  
  for (SizeType i = rowIndex + 1; i < m_rowOffsets.size(); ++i)
    m_rowOffsets[i]++;
}

PetscMatrix
PetscMatrix
  :: load(std::istream & input)
{
  SizeType header, m, n, nz;
  
  petscio::readIndices( input, &header, 1);
  if (header != petscio::Header::MATRIX)
    throw LoadException() << "File is not Petsc Matrix";

  petscio::readIndices(input, &m,  1);
  petscio::readIndices(input, &n,  1);
  petscio::readIndices(input, &nz,  1);
	
  PetscMatrix result(m, n);

  if (nz == SizeType(-1))
  { 
    throw LoadException() << "Petsc matrix is in dense format.";
    // matrix is dense
    // for (i = 0; i < m * n; ++i)
    //  fwriteScalar( freadScalar(input), output);
  }
  else
  { // matrix is sparse
    // read the number of non-zeros per row into m_rowOffsets
    result.m_rowOffsets.resize(m+1, 0u);
    petscio::readIndices( input, & result.m_rowOffsets[1], m);
    for (SizeType i = 0; i < m; ++i)
      result.m_rowOffsets[i+1] += result.m_rowOffsets[i];

    if (result.m_rowOffsets.back() != nz)
      throw LoadException() << "Row index information is inconsistent.";

    // read the columns indices
    result.m_columnIndices.resize( nz );
    petscio::readIndices( input, &result.m_columnIndices[0], nz);

    // read the values
    result.m_nonZeros.resize(nz);
    petscio::readReal(input, &result.m_nonZeros[0], nz);
  }

  return result;
}

void
PetscMatrix
  :: save( std::ostream & output) const
{
  const SizeType header = petscio::Header::MATRIX;
  const SizeType nz = m_nonZeros.size();

  petscio::writeIndices( output, & header, 1);
  petscio::writeIndices( output, & m_rows, 1);
  petscio::writeIndices( output, & m_columns, 1);
  petscio::writeIndices( output, & nz, 1);

  for (SizeType i = 0; i < m_rows; ++i)
  {
    SizeType nnz 
      = i+1 < m_rowOffsets.size() ? 
        m_rowOffsets[i+1] - m_rowOffsets[i] : 
	0;
    petscio::writeIndices( output, & nnz, 1);
  }

  petscio::writeIndices( output, &m_columnIndices[0], nz);

  for (SizeType i = 0; i < nz; ++i)
    petscio::writeReal(output, m_nonZeros[i] );
}


}
