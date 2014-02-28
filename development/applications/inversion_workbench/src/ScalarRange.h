#ifndef INVERSION_SCALAR_RANGE_H
#define INVERSION_SCALAR_RANGE_H

#include <cassert>
#include <cmath>

/// A one-dimensional range.
class ScalarRange
{
public:
   /// The range starts at 'start' and ends at 'end' (inclusive) 
   /// It can go from low to high or from high to low: 
   /// step size can be negative or positive.
   ScalarRange( double start, double end, double step)
    : m_start(start)
    , m_end(end)
    , m_step(step)
    , m_currentValue(m_start)
   {
      assert( m_step < 0.0 || m_step > 0.0 );
   }

   void   reset()          { m_currentValue = m_start; }
   double getValue() const { return m_currentValue; }
   void   nextValue()      { m_currentValue += m_step; }
   void   lastValue()      { m_currentValue = m_end; }

   bool isPastEnd() const 
   { 
      return m_step < 0.0 && m_currentValue < m_end
          || m_step > 0.0 && m_currentValue > m_end;
   }

private:
   double m_start, m_end, m_step;
   double m_currentValue;
};



#endif
