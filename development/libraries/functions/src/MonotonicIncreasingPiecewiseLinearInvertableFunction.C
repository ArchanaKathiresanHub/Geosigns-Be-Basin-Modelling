#include "MonotonicIncreasingPiecewiseLinearInvertableFunction.h"

#ifdef DEBUG
#include "enableGDBBreak.h"
#endif

#include <vector>
#include <algorithm>
#include <assert.h>

using std::vector;
using std::upper_bound;
using std::make_pair;

namespace functions {

MonotonicIncreasingPiecewiseLinearInvertableFunction::
   MonotonicIncreasingPiecewiseLinearInvertableFunction(const vector<element_type>& xToY):
   m_xToY(xToY)
{
   assert(m_xToY.size() != 1);
}

template <typename ForwardIterator>
MonotonicIncreasingPiecewiseLinearInvertableFunction::
   MonotonicIncreasingPiecewiseLinearInvertableFunction(ForwardIterator iter, ForwardIterator last)
{
   /*
  for (; iter != last; ++iter, ++j)
     m_xToY.push_back(element_type((*first).first,(*first).second));

   assert(m_xToY.size() != 1);
   */
}

// MonotonicIncreasingPiecewiseLinearInvertableFunction::MonotonicIncreasingPiecewiseLinearInvertableFunction
//    (const MonotonicIncreasingPiecewiseLinearInvertableFunction& rhs)
// {
//    m_xToY = rhs.m_xToY;
//    assert(m_xToY.size() != 1);
// }

Tuple2<MonotonicIncreasingPiecewiseLinearInvertableFunction::element_type> 
   MonotonicIncreasingPiecewiseLinearInvertableFunction::piece(int index) const
{
  assert(0 <= index && index < m_xToY.size()-1);
  return tuple(m_xToY[index], m_xToY[index+1]); 
}

const MonotonicIncreasingPiecewiseLinearInvertableFunction::element_type&  
   MonotonicIncreasingPiecewiseLinearInvertableFunction::begin(int index) const
{
  assert(0 <= index && index < m_xToY.size()-1);
  return m_xToY[index];
}

const MonotonicIncreasingPiecewiseLinearInvertableFunction::element_type&  
   MonotonicIncreasingPiecewiseLinearInvertableFunction::end(int index) const
{
  assert(0 <= index && index < m_xToY.size()-1);
  return m_xToY[index+1];
}

struct CompareX
{
   double m_x;

   CompareX(const double& x):
      m_x(x)
   {}

   bool operator<(const MonotonicIncreasingPiecewiseLinearInvertableFunction::element_type& xy) const
   {
      return m_x < xy[0];
   }
};

double MonotonicIncreasingPiecewiseLinearInvertableFunction::operator()(const double& x) const
{
#ifdef DEBUG
   enableGDBBreak::enableBreak1();
#endif

   assert(m_xToY.size() > 1);

   if (x <= m_xToY.front()[0])
     return m_xToY.front()[1];

   if (x >= m_xToY.back()[0])
     return m_xToY.back()[1];

   data_type::const_iterator j = upper_bound(
      m_xToY.begin(), m_xToY.end(), CompareX(x));

   assert(j != m_xToY.begin());
   data_type::const_iterator i = j; --i;

   if (j == m_xToY.end()) {
      assert(x == (*i)[0]);
      return (*i)[1];
   }
   else {
      double dist = (*j)[0] - (*i)[0];
      assert(dist > 0.0);
      double frac = (x - (*i)[0]) / dist;
      return (1.0 - frac) * (*i)[1] + frac * (*j)[1];
   }
}

struct CompareY
{
   double m_y;

   CompareY(const double& y):
      m_y(y)
   {}

   bool operator<(const MonotonicIncreasingPiecewiseLinearInvertableFunction::element_type& xy) const
   {
      return m_y < xy[1];
   }
};

double MonotonicIncreasingPiecewiseLinearInvertableFunction::invert(const double& y) const
{
#ifdef DEBUG
   enableGDBBreak::enableBreak2();
#endif

   assert(m_xToY.size() > 1);

   if (y <= m_xToY.front()[1])
     return m_xToY.front()[0];

   if (y >= m_xToY.back()[1])
     return m_xToY.back()[0];

   // equal_range instead of upper_bound ?
   data_type::const_iterator j = upper_bound(m_xToY.begin(), 
     m_xToY.end(), CompareY(y));

   assert(j != m_xToY.begin());
   data_type::const_iterator i = j; --i;

   if (j == m_xToY.end()) {
      assert(y == (*i)[1]);
      return (*i)[0];
   }
   else {
      double dist = (*j)[1] - (*i)[1];
      assert(dist > 0.0);
      double frac = (y - (*i)[1]) / dist;
      return (1.0 - frac) * (*i)[0] + frac * (*j)[0];
   }
}

MonotonicIncreasingPiecewiseLinearInvertableFunction& MonotonicIncreasingPiecewiseLinearInvertableFunction::
   shiftXBy(const double& value)
{
   if (value != 0.0)
      for (data_type::iterator i = m_xToY.begin(); i != m_xToY.end(); ++i)
	 (*i)[0] += value;
   return *this;
}

MonotonicIncreasingPiecewiseLinearInvertableFunction& MonotonicIncreasingPiecewiseLinearInvertableFunction::
   widenXBy(const double& value)
{
   assert(value > 0.0);
   if (value != 1.0)
      for (data_type::iterator i = m_xToY.begin(); i != m_xToY.end(); ++i)
	 (*i)[0] *= value;
   return *this;
}

MonotonicIncreasingPiecewiseLinearInvertableFunction& MonotonicIncreasingPiecewiseLinearInvertableFunction::
   operator+=(const Tuple2<double>& value)
{
   for (data_type::iterator i = m_xToY.begin(); i != m_xToY.end(); ++i)
      (*i) += value;
   return *this;
}

MonotonicIncreasingPiecewiseLinearInvertableFunction& MonotonicIncreasingPiecewiseLinearInvertableFunction::
   operator*=(const Tuple2<double>& value)
{
   assert(value[0] > 0.0 && value[1] > 0.0);
   for (data_type::iterator i = m_xToY.begin(); i != m_xToY.end(); ++i)
      (*i) *= value;
   return *this;
}

MonotonicIncreasingPiecewiseLinearInvertableFunction operator+(const 
  MonotonicIncreasingPiecewiseLinearInvertableFunction& first, const Tuple2<double>& second) 
{
  MonotonicIncreasingPiecewiseLinearInvertableFunction result(first);
  return result += second;
}

MonotonicIncreasingPiecewiseLinearInvertableFunction operator-(const 
  MonotonicIncreasingPiecewiseLinearInvertableFunction& first, const Tuple2<double>& second) 
{
  MonotonicIncreasingPiecewiseLinearInvertableFunction result(first);
  return result -= second;
}

MonotonicIncreasingPiecewiseLinearInvertableFunction operator*(const 
  MonotonicIncreasingPiecewiseLinearInvertableFunction& first, const Tuple2<double>& second) 
{
  MonotonicIncreasingPiecewiseLinearInvertableFunction result(first);
  return result *= second;
}

MonotonicIncreasingPiecewiseLinearInvertableFunction operator/(const 
  MonotonicIncreasingPiecewiseLinearInvertableFunction& first, const Tuple2<double>& second) 
{
  MonotonicIncreasingPiecewiseLinearInvertableFunction result(first);
  return result /= second;
}

MonotonicIncreasingPiecewiseLinearInvertableFunction shiftX(const 
  MonotonicIncreasingPiecewiseLinearInvertableFunction& first, const double& second) 
{
  MonotonicIncreasingPiecewiseLinearInvertableFunction result(first);
  return result.shiftXBy(second);
}

MonotonicIncreasingPiecewiseLinearInvertableFunction widenX(const 
  MonotonicIncreasingPiecewiseLinearInvertableFunction& first, const double& second) 
{
  MonotonicIncreasingPiecewiseLinearInvertableFunction result(first);
  return result.widenXBy(second);
}

} // namespace functions 
