#ifndef HPC_CUDACSRMATRIX_H
#define HPC_CUDACSRMATRIX_H

#include "cudaarray.h"
#include "cudamatdescr.h"

#include <cusparse_v2.h>

namespace hpc {

class CudaVector;
class PetscMatrix;

template <typename, typename > 
class ILU;

class CudaCSRMatrix
{ 
  friend class CudaHYBMatrix;
  friend class ILU<CudaCSRMatrix, CudaVector>;

public:
  typedef int SizeType;
  typedef double ValueType;

  CudaCSRMatrix( const PetscMatrix & matrix);
  void multiply( ValueType alpha, const CudaVector & x, ValueType beta, CudaVector & y) const;

  SizeType rows() const
  { return m_rows; }

  SizeType columns() const
  { return m_columns; }


  // Dereference
  // Proxy-row object
  struct Row 
  {
  public:
    ValueType operator[](SizeType column ) const;

  private:
    friend class CudaCSRMatrix;
    Row(const CudaCSRMatrix * matrix, SizeType rowBegin, SizeType rowEnd)
      : m_matrix(matrix), m_rowBegin(rowBegin), m_rowEnd(rowEnd)
    {}

    const CudaCSRMatrix * m_matrix;
    SizeType m_rowBegin, m_rowEnd;
  };

  Row operator[](SizeType row) const
  { return Row( this, m_rowOffsets[row], m_rowOffsets[row+1]); }

  // Iterator
  struct Element
  {
    SizeType row, column;
    ValueType value;
  };

  class Iterator
  { friend class CudaCSRMatrix;
  public:
    Iterator()
      : m_matrix(0)
      , m_rowIndex(0)
      , m_elemIndex(0)
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
    { return this->m_elemIndex == other.m_elemIndex; }

    bool operator!=(const Iterator & other) const
    { return this->m_elemIndex != other.m_elemIndex; }


  private:
    Iterator(const CudaCSRMatrix * matrix, SizeType rowIndex, SizeType elemIndex)
      : m_matrix(matrix)
      , m_rowIndex(rowIndex)
      , m_elemIndex(elemIndex)
    {}

    void increment()
    {
      ++m_elemIndex;
      if (m_matrix->m_rowOffsets[m_rowIndex+1] == m_elemIndex)
	++m_rowIndex;
    }
    
    void decrement()
    {
      --m_elemIndex;
      if (m_matrix->m_rowOffsets[m_rowIndex] > m_elemIndex)
	--m_rowIndex;
    }

    Element dereference() const
    {
      Element result 
	= { m_rowIndex, m_matrix->m_columnIndices[m_elemIndex], m_matrix->m_nonZeros[m_elemIndex] };

      return result;
    }

    const CudaCSRMatrix * m_matrix;
    SizeType m_rowIndex;
    SizeType m_elemIndex;
  };

  Iterator begin() const
  { return Iterator( this, 0, 0); }

  Iterator end() const
  { return Iterator( this, m_rows, m_nonZeros.size()); }

private:
  SizeType m_rows, m_columns;
  CudaMatrixDescription m_matdescr;
  CudaArray<SizeType> m_rowOffsets;
  CudaArray<SizeType> m_columnIndices;
  CudaArray<ValueType > m_nonZeros;
};



}


#endif
