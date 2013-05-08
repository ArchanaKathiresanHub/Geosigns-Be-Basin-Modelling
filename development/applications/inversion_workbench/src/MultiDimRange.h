#ifndef INVERSION_MULTIDIMRANGE_H
#define INVERSION_MULTIDIMRANGE_H

#include "ScalarRange.h"
#include <vector>

/// A multi-dimensional range.
class MultiDimRange
{
public:
   MultiDimRange( const std::vector< ScalarRange > & ranges);

   void reset();
   void getValue(std::vector< double > & values) const;
   void nextValue();
   bool isPastEnd() const ;

private:
   std::vector< ScalarRange > m_ranges;
   bool m_isPastEnd;
};



#endif
