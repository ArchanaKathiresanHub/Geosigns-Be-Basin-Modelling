#include "coomatrix.h"
#include "petscvector.h"

#include <cassert>
#include <mkl.h>
#include <mkl_spblas.h>

namespace hpc
{

COOMatrix
  :: COOMatrix( SizeType rows, SizeType columns)
  : m_rows(rows), m_columns(columns)
  , m_rowIndices(), m_columnIndices(), m_values()
{}


void
COOMatrix
  :: insert( SizeType row, SizeType column, ValueType value)
{
  if (m_rowIndices.empty() || row == m_rowIndices.back() && column > m_columnIndices.back() 
      || row > m_rowIndices.back() )
  {
    // then the insertion is in Row-Major order
    m_rowIndices.push_back( row );
    m_columnIndices.push_back(column);
    m_values.push_back( value);
  }
  else
  { // the insertion is not in in Row-Major order, but we want to have that

    SizeType rowBegin = std::distance( m_rowIndices.begin()
	, std::lower_bound( m_rowIndices.begin(), m_rowIndices.end(), row)
	);

    SizeType rowEnd = std::distance( m_rowIndices.begin()
	, std::upper_bound( m_rowIndices.begin(), m_rowIndices.end(), row)
	);


    SizeType index = std::distance( m_columnIndices.begin()
	, std::lower_bound( m_columnIndices.begin() + rowBegin, m_columnIndices.begin() + rowEnd, column)
	);

    if (index < m_columnIndices.size() && m_columnIndices[index] == column && m_rowIndices[index] == row)
    { // then overwrite the value
      m_values[index] = value;
    }
    else 
    { // insert the value
      m_rowIndices.insert( m_rowIndices.begin() + index, row);
      m_columnIndices.insert( m_columnIndices.begin() + index, column);
      m_values.insert( m_values.begin() + index, value);
    }
  }
}

void
COOMatrix
  :: multiply( ValueType alpha, const PetscVector & x, ValueType beta, PetscVector & y) const
{
  assert( x.rows() == this->columns() );
  assert( y.rows() == this->rows() );

  char transa = 'N';
  char matdescra[6] = "GLNC";
  SizeType rows = this->rows();
  SizeType columns = this->columns();
  SizeType nnz = m_values.size();
  mkl_dcoomv( &transa,
      & rows,
      & columns,
      &alpha,
      matdescra,
      const_cast<double *>( &m_values[0] ),
      const_cast<int32_t *>( &m_rowIndices[0]),
      const_cast<int32_t *>( &m_columnIndices[0]),
      &nnz,
      const_cast<double *>(&x[0]),
      &beta,
      &y[0]
  );
}



}
