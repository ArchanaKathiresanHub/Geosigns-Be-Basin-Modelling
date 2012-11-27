#ifndef HPC_PETSCMATRIX_HPP
#define HPC_PETSCMATRIX_HPP

#include <iosfwd>
#include <vector>

#include "generalexception.h"

namespace hpc
{


  class PetscMatrix
  {
  public:
    typedef uint32_t SizeType;
    typedef double  ValueType;

    // Construction
    template <typename Iterator>
    PetscMatrix(SizeType rows, SizeType columns, Iterator begin, Iterator end) 
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
	const SizeType * index = std::lower_bound( m_colIndices, m_colIndices + m_cols, column);
	if (*index == column)
	  return m_row[ index - m_colIndices ];
	else
	  return 0.0;
      }

    private:
      friend class PetscMatrix;
      Row( SizeType cols, const ValueType * row, const SizeType * colIndices)
	: m_cols(cols), m_row(row), m_colIndices(colIndices)
      {}

      SizeType m_cols;
      const ValueType * m_row;
      const SizeType * m_colIndices;
    };

    Row operator[](SizeType row) const
    { 
      return Row( 
	  m_rowOffsets[row+1] - m_rowOffsets[row], 
	  &m_nonZeros[ m_rowOffsets[row] ], 
	  &m_columnIndices[ m_rowOffsets[row]]
	  )  ;
    }

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

      Element operator*()
      { return dereference(); }

      bool operator==(const Iterator & other)
      { return this->m_elemIndex == other.m_elemIndex; }

      bool operator!=(const Iterator & other)
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

    const SizeType * rowOffsets() const
    { return &m_rowOffsets[0]; }

    const SizeType * columnIndices() const
    { return &m_columnIndices[0]; }

    const ValueType * nonZeros() const
    { return &m_nonZeros[0]; }

    const SizeType numberNonZeros() const
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
