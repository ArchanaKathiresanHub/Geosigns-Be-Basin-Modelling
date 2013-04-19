#ifndef HPC_DIAMATRIX_H
#define HPC_DIAMATRIX_H

#include <vector>

namespace hpc
{

class PetscVector;

class DIAMatrix
{
public:
  typedef int32_t SizeType;
  typedef double  ValueType;

  DIAMatrix(SizeType rows, SizeType columns);

  template <class DiagIterator >
  DIAMatrix(SizeType rows, SizeType columns
    , DiagIterator diagBegin, DiagIterator diagEnd
    )
    : m_rows(rows), m_columns(columns)
    , m_distances(diagBegin, diagEnd)
    , m_values(m_distances.size()*m_rows)
    , m_distanceIndex()
  {
    initDistanceIndex();
  }

  template <typename COOIterator, typename DiagIterator >
  DIAMatrix(SizeType rows, SizeType columns
    , DiagIterator diagsBegin, DiagIterator diagsEnd 
    , COOIterator elemsBegin, COOIterator elemsEnd
    )
    : m_rows(rows), m_columns(columns)
    , m_distances(diagsBegin, diagsEnd)
    , m_values(m_distances.size()*m_rows)
    , m_distanceIndex()
  {
    initDistanceIndex();

    insert( elemsBegin, elemsEnd); 
  }
  
  SizeType rows() const
  { return m_rows; }

  SizeType columns() const
  { return m_columns; }

  bool contains( SizeType row, SizeType column) const
  { 
    return row >= 0 && row < m_rows
       	&& column >= 0 && column < m_columns 
	&& distanceIndex( diagonalNr(row, column) ) != -1
	;
  }

  template <typename It>
  void insert( It begin, It end)
  {
    for (It i = begin ; i != end; ++i)
      insert( (*i).row, (*i).column, (*i).value );
  }

  void insert( SizeType row, SizeType column, ValueType value);


  class Row
  { friend class DIAMatrix;
  public:

    ValueType operator[](SizeType column) const
    { 
      const SizeType d = m_matrix->diagonalNr(m_row, column); 
      const SizeType rows = m_matrix->m_rows;
      const SizeType di = m_matrix->distanceIndex( d );
      if (di == -1)
	return 0.0;
      else
        return m_matrix->m_values[ di  * rows + m_row];
    }

  private:
    Row( const DIAMatrix * matrix, SizeType row)
      : m_matrix(matrix), m_row(row)
    {}

    const DIAMatrix * m_matrix;
    SizeType m_row;
  };

  Row operator[](SizeType row) const
  { 
    initDistanceIndex();
    return Row(this, row); 
  }

  struct Element
  {
    SizeType row, column;
    ValueType value;
  };

  class Iterator
  { friend class DIAMatrix;
  public:
    Iterator()
      : m_matrix(0)
      , m_index(0)
    {}		       

    Iterator & operator++() // prefix
    { increment(); return *this; }

    Iterator operator++(int)  // postfix
    { Iterator copy(*this); increment(); return copy; }

    Iterator & operator--() // prefix
    { decrement(); return *this; }

    Iterator operator--(int)  // postfix
    { Iterator copy(*this); decrement(); return copy; }

    Element operator*() const
    { return dereference(); }

    bool operator==(const Iterator & other) const
    { return this->m_index == other.m_index; }

    bool operator!=(const Iterator & other) const
    { return this->m_index != other.m_index; }

  private:
    Iterator(const DIAMatrix * matrix, SizeType index)
      : m_matrix(matrix)
      , m_index(index)
    {}

    void increment()
    {
      ++m_index;
  
      const SizeType rows = m_matrix->m_rows;
      if ( unsigned(m_index / rows) < m_matrix->m_distances.size())
      {
	if ( m_index % rows == 0 )
	  m_index += m_matrix->diagonalStart( diagonal() );
	else if (m_index % rows == m_matrix->diagonalEnd( diagonal()))
	  m_index += (rows - (m_index % rows)); 
      }
    }
    
    void decrement()
    {
      --m_index;

      if ( m_index > 0)
      {
	const SizeType rows = m_matrix->m_rows;
	if ( m_index % rows == rows-1 )
	  m_index -= (rows - m_matrix->diagonalEnd( diagonal() ));
	else if (m_index % rows == m_matrix->diagonalStart( diagonal() ) - 1)
	  m_index -= (m_index % rows);
      }
    }

    Element dereference() const
    {
      Element result 
	= { row()
          , column()
          , m_matrix->m_values[m_index]
          };

      return result;
    }

    SizeType row() const
    { return m_index % m_matrix->m_rows; }
  
    SizeType column() const
    { return diagonal() + row(); }

    SizeType diagonal() const
    { return m_matrix->m_distances[ m_index / m_matrix->m_rows ]; }

    const DIAMatrix * m_matrix;
    SizeType m_index;
  };

  Iterator begin() const
  { return Iterator( this, diagonalStart( m_distances.front()  ) ); }

  Iterator end() const
  { return Iterator( this, m_rows * m_distances.size() ); }

  void multiply( ValueType alpha, const PetscVector & x, ValueType beta, PetscVector & y) const;

private:
  static SizeType diagonalNr( SizeType row, SizeType column)
  { return column - row; }

  SizeType nDiagonals() const
  { return m_rows + m_columns -1;}

  SizeType diagonalLength(SizeType diagonalNr) const
  { 
    if (diagonalNr <= 0 )
      return std::min( m_columns, diagonalNr + m_rows );
    else
      return std::min( m_rows, m_columns - diagonalNr);
  }
 
  SizeType diagonalStart(SizeType diagonalNr) const
  { return std::max( -diagonalNr, 0); }

  SizeType diagonalEnd(SizeType diagonalNr) const
  { return std::min( m_columns - diagonalNr, m_rows); }

  SizeType maxDiagonalLength() const
  { return std::min( m_rows, m_columns); }

  SizeType distanceIndex(SizeType diagonalNr) const
  { return m_distanceIndex[ diagonalNr + m_rows - 1 ]; }

  void initDistanceIndex() const;

  // core data
  SizeType m_rows, m_columns;
  std::vector< SizeType > m_distances;
  std::vector< ValueType > m_values;
 
  // data which makes random access fast
  mutable std::vector< SizeType > m_distanceIndex;
};


}

#endif
