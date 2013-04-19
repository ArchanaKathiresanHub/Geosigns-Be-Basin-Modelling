#ifndef HPC_PETSCVECTOR_H
#define HPC_PETSCVECTOR_H

#include <vector>
#include <iosfwd>

#include "generalexception.h"

namespace hpc
{

class PetscVector
{
public:
  typedef int32_t SizeType;
  typedef double ValueType;

  PetscVector()
    : m_values()
  {}

  struct LoadException : BaseException<LoadException> {};
  static PetscVector load( std::istream & );

  void save(std::ostream & ) const;

  void resize( SizeType rows)
  { m_values.resize(rows, 0.0); }

  SizeType rows() const
  { return m_values.size(); }

  ValueType & operator[](SizeType i )
  { return m_values[i]; }

  const ValueType & operator[](SizeType i) const
  { return m_values[i]; }

  typedef std::vector<ValueType>::const_iterator
    Iterator;

  Iterator begin() const
  { return m_values.begin(); }

  Iterator end() const
  { return m_values.end(); }

private:
  std::vector<ValueType> m_values;
};


}

#endif
