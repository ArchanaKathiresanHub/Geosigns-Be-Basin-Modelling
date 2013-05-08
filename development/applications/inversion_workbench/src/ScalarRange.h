#ifndef INVERSION_SCALAR_RANGE_H
#define INVERSION_SCALAR_RANGE_H

/// A one-dimensional range.
class ScalarRange
{
public:
   /// The range starts at 'start' and ends at 'end' (inclusive) 
   ScalarRange( double start, double end, double step)
    : m_start(start)
    , m_end(end)
    , m_step(step)
    , m_currentValue(m_start)
   {}

   void reset()
   { m_currentValue = m_start; }

   double getValue() const
   { return m_currentValue; }

   void nextValue()
   { m_currentValue += m_step; }

   bool isPastEnd() const 
   { return m_currentValue > m_end; }

private:
   double m_start, m_end, m_step;
   double m_currentValue;
};



#endif
