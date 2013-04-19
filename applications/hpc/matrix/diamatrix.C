#include "diamatrix.h"
#include "petscvector.h"

#include <algorithm>
#include <cassert>

#include <mkl.h>
#include <mkl_spblas.h>

namespace hpc
{


DIAMatrix
  :: DIAMatrix( SizeType rows, SizeType columns)
  : m_rows(rows), m_columns(columns)
  , m_distances()
  , m_values()
  , m_distanceIndex()
{

}


void
DIAMatrix
  :: initDistanceIndex() const
{
  if (m_distanceIndex.empty())
  {
    m_distanceIndex.resize( nDiagonals(), -1 );
    for (unsigned i = 0; i < m_distances.size()  ; ++i)
      m_distanceIndex[ m_distances[ i ] + m_rows - 1 ] = i;
  }
}

void
DIAMatrix
  :: insert( SizeType row, SizeType column, ValueType value)
{
  initDistanceIndex();

  if (distanceIndex( diagonalNr(row, column) ) == -1)
  { 
    // then there is no storage allocated for this diagonal  
    // where should this new diagonal be added?
    SizeType di
      = std::distance( 
	  m_distances.begin(), 
	  std::lower_bound( m_distances.begin(), m_distances.end(), diagonalNr(row, column))
	);

    // insert a new distance 
    m_distances.insert( m_distances.begin() + di , diagonalNr(row, column));

    // insert a diagonal full of zeros at the appropriate place in the m_values array.
    m_values.insert( 
	m_values.begin() + m_rows * di,
        m_rows,
	0.0
    );

    // update the distanceIndex
    m_distanceIndex[ diagonalNr(row, column) + m_rows - 1] = di;
    for (unsigned i = di+1; i < m_distances.size(); ++i)
      m_distanceIndex[ m_distances[i] + m_rows - 1]++;

    // and continue as normal
  }
  
  // then the column for this diagonal is given by distanceIndex
  m_values[ distanceIndex( diagonalNr(row, column) ) * m_rows + row  ] = value;
}

void
DIAMatrix
  :: multiply( ValueType alpha, const PetscVector & x, ValueType beta, PetscVector & y) const
{
  assert( x.rows() == this->columns() );
  assert( y.rows() == this->rows() );

  char transa = 'N';
  char matdescra[6] = "GLNC";
  SizeType rows = this->rows();
  SizeType columns = this->columns();
  SizeType diagonals = m_distances.size();
  mkl_ddiamv( &transa,
      & rows,
      & columns,
      &alpha,
      matdescra,
      const_cast<double *>( &m_values[0] ),
      & rows,
      const_cast<int32_t *>( &m_distances[0]),
      & diagonals, 
      const_cast<double *>(&x[0]),
      &beta,
      &y[0]
  );
}




}
