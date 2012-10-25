#ifndef sun

// "../../multi_array/MultiArrayBase.h", line 41: Error: Ambiguous partial specialization for multi_array::CloseOpenInterval<1, int>, multi_array::CloseOpenInterval and multi_array::CloseOpenInterval.

template <typename ENTRY>
class CloseOpenInterval<1,ENTRY>: public Interval<1,ENTRY> 
{

#define UNCOUNTABLE ENTRY
#include "multi_array/CloseOpenInterval_UNCOUNTABLE_1.h"
#undef UNCOUNTABLE

};

// It seems that there is a problem with define COUNTABLE int as
// including these statements leads to the error:
// "../../multi_array/CloseOpenInterval_UNCOUNTABLE_1.h", line 19: Error: multi_array::Interval<multi_array::ENTRY> is not a direct base class of multi_array::CloseOpenInterval<1, int>.

template <>
class CloseOpenInterval<1,int>: public Interval<1,int> 
{

#define COUNTABLE int
#include "multi_array/CloseOpenInterval_COUNTABLE_1.h"
#undef COUNTABLE

#define UNCOUNTABLE int
#include "multi_array/CloseOpenInterval_UNCOUNTABLE_1.h"
#undef UNCOUNTABLE

};

template <>
class CloseOpenInterval<1,long>: public Interval<1,long> 
{

#define COUNTABLE long
#include "multi_array/CloseOpenInterval_COUNTABLE_1.h"
#undef COUNTABLE

#define UNCOUNTABLE long
#include "multi_array/CloseOpenInterval_UNCOUNTABLE_1.h"
#undef UNCOUNTABLE

};

template <>
class CloseOpenInterval<1,unsigned int>: public Interval<1,unsigned int> 
{

#define COUNTABLE unsigned int
#include "multi_array/CloseOpenInterval_COUNTABLE_1.h"
#undef COUNTABLE

#define UNCOUNTABLE unsigned int
#include "multi_array/CloseOpenInterval_UNCOUNTABLE_1.h"
#undef UNCOUNTABLE

};

#endif




