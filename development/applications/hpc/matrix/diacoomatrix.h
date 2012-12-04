#ifndef HPC_DIACOOMATRIX_H
#define HPC_DIACOOMATRIX_H

#include "diamatrix.h"
#include "coomatrix.h"


namespace hpc
{


class DIACOOMatrix
{
public:
  typedef int32_t SizeType;
  typedef double ValueType;

  template <typename DiagonalsIterator, typename ElementIterator>
  DIACOOMatrix( SizeType rows, SizeType columns
      , DiagonalsIterator diagBegin, DiagonalsIterator diagEnd
      , ElementIterator elemsBegin, ElementIterator elemsEnd
      )
    : m_diaMatrix(rows, columns, diagBegin, diagEnd)
    , m_cooMatrix(rows, columns)
  {
    std::vector< int > isMajorDiagonal( rows + columns - 1, 0);
    for (DiagonalsIterator d = diagBegin; d != diagEnd; ++d)
      isMajorDiagonal[*d + rows - 1] = 1;

    for (ElementIterator e = elemsBegin; e != elemsEnd; ++e)
    {
      SizeType diagonal = (*e).column - (*e).row;

      if (isMajorDiagonal[ diagonal + rows - 1])
	m_diaMatrix.insert( (*e).row, (*e).column, (*e).value);
      else
	m_cooMatrix.insert( (*e).row, (*e).column, (*e).value);
    }
  }

  template <typename It>
  static DIACOOMatrix create( SizeType rows, SizeType columns, SizeType diagonals
      , It begin, It end 
      )
  {
    typedef SizeType Frequency;
    typedef SizeType Diagonal;
    std::vector< std::pair< Frequency, Diagonal > > diagonalFrequency( rows + columns - 1);
    for (SizeType i = 0; i < rows + columns - 1; ++i)
    {
      diagonalFrequency[i].first = 0;
      diagonalFrequency[i].second = i - rows + 1;
    }

    for (It i = begin; i != end; ++i)
      diagonalFrequency[ rows - 1 + (*i).column - (*i).row ].first ++;

    // get the Nth highest element where 'N' = 'diagonals'
    std::nth_element( diagonalFrequency.begin(), diagonalFrequency.begin() + diagonals, diagonalFrequency.end()
	, std::greater< std::pair<Frequency, Diagonal > >()
	);

    // Extract the N most frequent diagonals
    std::vector< SizeType > diagonalNrs( diagonals );
    for (SizeType i = 0; i < diagonals; ++i)
      diagonalNrs[i] = diagonalFrequency[i].second;

    return DIACOOMatrix( rows, columns, diagonalNrs.begin(), diagonalNrs.end(), begin, end);
  }

  SizeType rows() const
  { return m_diaMatrix.rows(); }

  SizeType columns() const
  { return m_diaMatrix.columns(); }


  class Row
  { friend class DIACOOMatrix;
  public:
    ValueType operator[](SizeType column) const
    { return m_diarow[column] + m_coorow[column]; }

  private:
    Row( DIAMatrix::Row diarow, COOMatrix::Row coorow)
      : m_diarow(diarow)
      , m_coorow(coorow)
    {}

    DIAMatrix::Row m_diarow;
    COOMatrix::Row m_coorow;
  };

  Row operator[](SizeType row) const
  { return Row( m_diaMatrix[row], m_cooMatrix[row] ); }

  struct Element
  {
    SizeType row, column;
    ValueType value;
  };

  class Iterator
  { friend class DIACOOMatrix;
  public:
    Iterator()
      : m_matrix(0)
      , m_diaIt()
      , m_cooIt()
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
    { return this->m_diaIt == other.m_diaIt && this->m_cooIt == other.m_cooIt; }

    bool operator!=(const Iterator & other) const
    { return this->m_diaIt != other.m_diaIt || this->m_cooIt != other.m_cooIt; }

  private:
    Iterator(const DIACOOMatrix * matrix, DIAMatrix::Iterator diaIt, COOMatrix::Iterator cooIt)
      : m_matrix(matrix)
      , m_diaIt(diaIt)
      , m_cooIt(cooIt)
    {}

    void increment()
    {
      if (m_diaIt == m_matrix->m_diaMatrix.end())
	++m_cooIt;
      else
	++m_diaIt;
    }
    
    void decrement()
    {
      if (m_diaIt == m_matrix->m_diaMatrix.end() && m_cooIt != m_matrix->m_cooMatrix.begin() )
	--m_cooIt;
      else
	--m_diaIt;
    }

    Element dereference() const
    { 
      if (m_diaIt == m_matrix->m_diaMatrix.end())
      {
	Element result = { (*m_cooIt).row, (*m_cooIt).column, (*m_cooIt).value };
        return result;
      }
      else
      {
	Element result = { (*m_diaIt).row, (*m_diaIt).column, (*m_diaIt).value };
        return result;
      }
    }

    const DIACOOMatrix * m_matrix;
    DIAMatrix::Iterator m_diaIt;
    COOMatrix::Iterator m_cooIt;
  };

  Iterator begin() const
  { return Iterator( this, m_diaMatrix.begin(), m_cooMatrix.begin()  ); }

  Iterator end() const
  { return Iterator( this, m_diaMatrix.end(), m_cooMatrix.end() ); }

  void multiply( ValueType alpha, const PetscVector & x, ValueType beta, PetscVector & y) const
  {
    m_diaMatrix.multiply( alpha, x, beta, y);
    m_cooMatrix.multiply( alpha, x, 1.0, y);
  }


private:
  DIAMatrix m_diaMatrix;
  COOMatrix m_cooMatrix;
};


}

#endif

