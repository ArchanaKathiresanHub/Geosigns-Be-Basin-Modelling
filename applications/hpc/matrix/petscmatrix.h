#ifndef HPC_PETSCMATRIX_HPP
#define HPC_PETSCMATRIX_HPP

#include <iosfwd>
#include <vector>
#include <algorithm>

#include "generalexception.h"

namespace hpc
{
  class PetscVector;

  class PetscMatrix
  {
  public:
    typedef int32_t SizeType;
    typedef double  ValueType;

    // Construction
    template <typename It>
    PetscMatrix(SizeType rows, SizeType columns, It begin, It end) 
      : m_rows(rows)
      , m_columns(columns)
      , m_nonZeros()
      , m_columnIndices()
      , m_rowOffsets()
    {
      for (Iterator i = begin; i != end; ++i)
	insert( (*i).row, (*i).column, (*i).value);
    }

    // insertion
    void insert( SizeType rowIndex, SizeType columnIndex, ValueType value);

    // Input
    struct LoadException : BaseException<LoadException> {};
    static PetscMatrix load( std::istream & );

    // Output
    void save( std::ostream & ) const;

    // general functions
    SizeType rows() const
    { return m_rows; }

    SizeType columns() const
    { return m_columns; }

    // Dereference
    // Proxy-row object
    struct Row 
    {
    public:
      ValueType operator[](SizeType column ) const
      {
	const SizeType * columns = & m_matrix->m_columnIndices[0];
	const SizeType index 
	  = std::distance( 
	        columns,
	        std::lower_bound( columns + m_rowBegin, columns + m_rowEnd, column)
	      );

	if ( index < m_rowEnd && columns[index] == column)
	  return m_matrix->m_nonZeros[ index];
	else
	  return 0.0;
      }

    private:
      friend class PetscMatrix;
      Row(const PetscMatrix * matrix, SizeType rowBegin, SizeType rowEnd)
	: m_rowBegin(rowBegin), m_rowEnd(rowEnd), m_matrix(matrix)
      {}

      SizeType m_rowBegin, m_rowEnd;
      const PetscMatrix * m_matrix;
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
    { friend class PetscMatrix;
    public:
      Iterator()
	: m_rowOffsets(0)
	, m_columnIndices(0)
 	, m_nonZeros(0)
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
      Iterator(const SizeType * rowOffsets, const SizeType * columnIndices,
	  const ValueType * nonZeros, SizeType rowIndex, SizeType elemIndex)
	: m_rowOffsets(rowOffsets)
	, m_columnIndices(columnIndices)
	, m_nonZeros(nonZeros)
        , m_rowIndex(rowIndex)
        , m_elemIndex(elemIndex)
      {}
 
      void increment()
      {
	++m_elemIndex;
	if (m_rowOffsets[m_rowIndex+1] == m_elemIndex)
	  ++m_rowIndex;
      }
      
      void decrement()
      {
	--m_elemIndex;
	if (m_rowOffsets[m_rowIndex] > m_elemIndex)
    	  --m_rowIndex;
      }

      Element dereference() const
      {
	Element result 
	  = { m_rowIndex, m_columnIndices[m_elemIndex], m_nonZeros[m_elemIndex] };

	return result;
      }

      const SizeType * m_rowOffsets;
      const SizeType * m_columnIndices;
      const ValueType * m_nonZeros;
      SizeType m_rowIndex;
      SizeType m_elemIndex;
    };

    Iterator begin() const
    { return Iterator( &m_rowOffsets[0], &m_columnIndices[0], &m_nonZeros[0], 0, 0); }

    Iterator end() const
    { return Iterator( &m_rowOffsets[0], &m_columnIndices[0], &m_nonZeros[0], m_rows, m_nonZeros.size()); }


    void multiply( ValueType alpha, const PetscVector & x, ValueType beta, PetscVector & y) const;

    const SizeType * rowOffsets() const
    { return &m_rowOffsets[0]; }

    const SizeType * columnIndices() const
    { return &m_columnIndices[0]; }

    const ValueType * nonZeros() const
    { return &m_nonZeros[0]; }

    SizeType numberNonZeros() const
    { return m_nonZeros.size(); }

  private:
   // Create an empty matrix
    PetscMatrix(SizeType numberOfRows, SizeType numberOfColumns);

    SizeType m_rows, m_columns; // the number of rows and columns
    std::vector< SizeType > m_rowOffsets; // points to index in m_nonZeros & m_columnIndices where each row starts
    std::vector< SizeType > m_columnIndices; // the columns index of each non-zero
    std::vector< ValueType > m_nonZeros; // the non-zeros;
  };



}

#endif
