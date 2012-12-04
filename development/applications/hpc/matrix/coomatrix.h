#ifndef HPC_COOMATRIX_H
#define HPC_COOMATRIX_H

#include <vector>
#include <algorithm>

namespace hpc
{

class PetscVector;

class COOMatrix
{
public:
  typedef int32_t SizeType;
  typedef double  ValueType;

  COOMatrix(SizeType rows, SizeType columns);

  template <typename It>
  COOMatrix(SizeType rows, SizeType columns, It begin, It end)
    : m_rows(rows), m_columns(columns)
    , m_rowIndices(), m_columnIndices(), m_values()
  {
    for (It i = begin; i != end; ++i)
      insert( (*i).row, (*i).column, (*i).value );
  }    

  SizeType rows() const
  { return m_rows; }

  SizeType columns() const
  { return m_columns; }

  void insert( SizeType row, SizeType column, ValueType value); 

  class Row
  { friend class COOMatrix;
  public:
    ValueType operator[](SizeType column) const
    {
      SizeType i 
	= std::distance( m_matrix->m_columnIndices.begin()
	    , std::lower_bound( m_matrix->m_columnIndices.begin() + m_rowBegin
	      , m_matrix->m_columnIndices.begin() + m_rowEnd
	      , column
	      )
	    );

      if ( unsigned(i) < m_matrix->m_values.size() && m_matrix->m_columnIndices[i] == column)
        return m_matrix->m_values[ i ];
      else 
	return 0.0;
    }

  private:
    Row(const COOMatrix * matrix, SizeType rowBegin, SizeType rowEnd)
      : m_matrix(matrix)
      , m_rowBegin(rowBegin)
      , m_rowEnd(rowEnd)
    {}

    const COOMatrix * m_matrix;
    SizeType m_rowBegin, m_rowEnd;
  };

  Row operator[](SizeType row) const
  {
    return Row(this
	, std::distance( m_rowIndices.begin()
	  , std::lower_bound( m_rowIndices.begin(), m_rowIndices.end(), row)
	  )
	, std::distance( m_rowIndices.begin()
	  , std::upper_bound( m_rowIndices.begin(), m_rowIndices.end(), row)
	  )
	);
  }


  struct Element
  {
    SizeType row, column;
    ValueType value;
  };

  class Iterator
  { friend class COOMatrix;
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
    Iterator(const COOMatrix * matrix, SizeType index)
      : m_matrix(matrix)
      , m_index(index)
    {}

    void increment()
    {
      ++m_index;
    }
    
    void decrement()
    {
      --m_index;
    }

    Element dereference() const
    {
      Element result 
	= { m_matrix->m_rowIndices[m_index]
	  , m_matrix->m_columnIndices[m_index]
	  , m_matrix->m_values[m_index] 
	};

      return result;
    }

    const COOMatrix * m_matrix;
    SizeType m_index;
  };

  Iterator begin() const
  { return Iterator( this, 0); }

  Iterator end() const
  { return Iterator( this, m_rowIndices.size() ); }

  void multiply( ValueType alpha, const PetscVector & x, ValueType beta, PetscVector & y) const;

private:
  SizeType m_rows, m_columns;
  std::vector< SizeType > m_rowIndices;
  std::vector< SizeType > m_columnIndices;
  std::vector< ValueType > m_values;
};

}

#endif
