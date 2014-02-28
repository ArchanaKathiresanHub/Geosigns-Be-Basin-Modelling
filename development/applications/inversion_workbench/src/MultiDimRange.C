#include "MultiDimRange.h"
#include <cassert>

MultiDimRange::MultiDimRange( const std::vector< ScalarRange > & ranges) : m_ranges(ranges), m_isPastEnd(false)
{
   reset();
}

void MultiDimRange::reset()
{ 
   m_isPastEnd = m_ranges.empty();

   for (size_t i = 0; i < m_ranges.size(); ++i)
   {
      m_ranges[i].reset();
      if (m_ranges[i].isPastEnd())
         m_isPastEnd = true;
   }
}

void MultiDimRange::getValue(std::vector< double > & values) const
{ 
   assert(!m_isPastEnd);

   values.resize( m_ranges.size() );
   for (size_t i = 0; i < m_ranges.size(); ++i)
      values[i] = m_ranges[i].getValue();
}

void MultiDimRange::nextValue()
{
   for (size_t i = 0; i < m_ranges.size(); ++i)
   {
      m_ranges[i].nextValue();

      if ( !m_ranges[i].isPastEnd() )
         return;

      m_ranges[i].reset();
   }

   m_isPastEnd = true;
}

void MultiDimRange::lastValue()
{
   reset();
   for ( size_t i = 0; i < m_ranges.size(); ++i )
   {
      m_ranges[i].lastValue();
   }
}


bool MultiDimRange::isPastEnd() const 
{ 
   return m_isPastEnd;
}

