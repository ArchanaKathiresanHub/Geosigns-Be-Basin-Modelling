#ifndef HPC_CUDAVECTOR_H
#define HPC_CUDAVECTOR_H

#include "cudaarray.h"
#include "petscvector.h"

namespace hpc
{

class CudaVector
{
public:
  typedef int SizeType;
  typedef double ValueType;

  CudaVector(const PetscVector & x)
    : m_values( &x[0], &x[0] + x.rows() )
  {}

  CudaVector()
    : m_values()
  {}

  SizeType rows() const
  { return m_values.size() ;}

  ValueType * devicePointer()
  { return m_values.devicePointer(); }

  const ValueType * devicePointer() const
  { return m_values.devicePointer(); }

  ValueType operator[](SizeType i) const 
  { return m_values[i]; }


  class Iterator
  { friend class CudaVector;
  public:
    Iterator()
      : m_vector(0)
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

    ValueType operator*() const
    { return dereference(); }

    bool operator==(const Iterator & other) const
    { return this->m_index == other.m_index; }

    bool operator!=(const Iterator & other) const
    { return this->m_index != other.m_index; }


  private:
    Iterator(const CudaVector * vector, SizeType index )
      : m_vector(vector)
      , m_index(index)
    {}

    void increment()
    { ++m_index; }
    
    void decrement()
    { --m_index; }

    ValueType dereference() const
    { return (*m_vector)[m_index]; }

    const CudaVector * m_vector;
    SizeType m_index;
  };

  Iterator begin() const
  { return Iterator( this, 0); }

  Iterator end() const
  { return Iterator( this, rows() ); }

private:
  CudaArray<ValueType> m_values;
};

}

#endif
