#ifndef _FUNCTIONS_MONOTONICINCREASINGPIECEWISELINEARINVERTABLEFUNCTION_H_
#define _FUNCTIONS_MONOTONICINCREASINGPIECEWISELINEARINVERTABLEFUNCTION_H_

#include "InvertableFunction.h"
#include "Tuple2.h"

#include <vector>
#include <algorithm>
#include <assert.h>

namespace functions {

   class MonotonicIncreasingPiecewiseLinearInvertableFunction: public InvertableFunction
   {
   public:

      typedef Tuple2<double> element_type;
      typedef Tuple2<double> & element_ref;

   protected:

      typedef std::vector<element_type> data_type;
      data_type m_xToY;

   public:

      MonotonicIncreasingPiecewiseLinearInvertableFunction(const std::vector<element_type>& xToY);

      int size() const { return static_cast<int>(m_xToY.size()) - 1; }
      Tuple2<element_type> piece(int index) const;
      const element_type& begin(int index) const;
      const element_type& end(int index) const;

      double operator()(const double& x) const;
      double invert(const double& y) const;

      /// Calculate the index of the linear piece which contains pred.
      ///
      /// @return: -1 if all entries are larger than pred
      ///           0..size()-1 if pred is within the range
      ///           size() if pred is equal or larger than the last entry.
      template <typename Predicate>
      int findIndex(Predicate pred) const
      {
         assert(m_xToY.size() > 1);

         // This function returns the linear piece corresponding to pred.operator<(Tuple2<double>).
         // However, when pred indicates that the element one searches for lies beyond the
         // the last element, linear piece m_xToY.size() is returned, i.e. one more than size().
         // And likewise, if pred corresponds to the element just in front of m_xToY.front(),
         // value -1 is returned.
         if (!(pred < m_xToY.back()))
            return static_cast<int>(m_xToY.size());

         // maybe use equal_range instead of upper_bound ?
         data_type::const_iterator j = std::upper_bound(m_xToY.begin(), m_xToY.end(), pred);

         int result = static_cast<int>(j - m_xToY.begin());
         --result;
         assert(-1 <= result && result < size());
         return result;
      }

      MonotonicIncreasingPiecewiseLinearInvertableFunction& operator+=(const Tuple2<double>& value);

      MonotonicIncreasingPiecewiseLinearInvertableFunction& operator-=(const Tuple2<double>& value);

      MonotonicIncreasingPiecewiseLinearInvertableFunction& operator*=(const Tuple2<double>& value);

      MonotonicIncreasingPiecewiseLinearInvertableFunction& operator/=(const Tuple2<double>& value);

      MonotonicIncreasingPiecewiseLinearInvertableFunction& shiftXBy(const double& value);

      MonotonicIncreasingPiecewiseLinearInvertableFunction& widenXBy(const double& value);
   };

   MonotonicIncreasingPiecewiseLinearInvertableFunction operator+(const
     MonotonicIncreasingPiecewiseLinearInvertableFunction& first, const Tuple2<double>& second);

   MonotonicIncreasingPiecewiseLinearInvertableFunction operator-(const
     MonotonicIncreasingPiecewiseLinearInvertableFunction& first, const Tuple2<double>& second);

   MonotonicIncreasingPiecewiseLinearInvertableFunction operator*(const
     MonotonicIncreasingPiecewiseLinearInvertableFunction& first, const Tuple2<double>& second);

   MonotonicIncreasingPiecewiseLinearInvertableFunction operator/(const
     MonotonicIncreasingPiecewiseLinearInvertableFunction& first, const Tuple2<double>& second);

   MonotonicIncreasingPiecewiseLinearInvertableFunction shiftX(const
     MonotonicIncreasingPiecewiseLinearInvertableFunction& first, const double& second);

   MonotonicIncreasingPiecewiseLinearInvertableFunction widenX(const
     MonotonicIncreasingPiecewiseLinearInvertableFunction& first, const double& second);

} // namespace functions

#endif


