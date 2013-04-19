#ifndef HPC_MATRIX_H
#define HPC_MATRIX_H

#include <cmath>
#include <iostream>
#include <limits>

namespace hpc
{


class AlmostEqual
{
public:
  AlmostEqual( double epsilon)
    : m_epsilon(epsilon)
  {}

  bool operator()( double x, double y) const
  { return std::fabs( x - y ) < m_epsilon ; }

private:
  double m_epsilon;
};

class RelativlyEqual
{
public:
  RelativlyEqual( double epsilon)
    : m_epsilon(epsilon)
  {}

  bool operator()( double x, double y) const
  {
    double machineEps = std::numeric_limits<double>::epsilon();
    return std::abs( x - y ) / (machineEps + std::min(std::abs(x), std::abs(y))) < m_epsilon ; 
  }

private:
  double m_epsilon;
};

template <typename M, typename N >
bool matrixEqual( const M & a, const N & b )
{
  if (a.rows() != b.rows() || a.columns() != b.columns() )
    return false;


  for (typename M::Iterator i = a.begin(); i != a.end(); ++i)
    if ( b[ (*i).row ] [ (*i).column] != (*i).value  )
      return false;

  for (typename N::Iterator i = b.begin(); i != b.end(); ++i)
    if ( a[ (*i).row ] [ (*i).column] != (*i).value  )
      return false;
 
  return true;
}

}

#endif
