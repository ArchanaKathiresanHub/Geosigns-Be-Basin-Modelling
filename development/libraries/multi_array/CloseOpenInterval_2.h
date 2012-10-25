template <typename ENTRY>
class CloseOpenInterval<2,ENTRY>: public Interval<2,ENTRY> 
{
#define UNCOUNTABLE ENTRY
#include "multi_array/CloseOpenInterval_UNCOUNTABLE_2.h"
#undef UNCOUNTABLE

};

template <>
class CloseOpenInterval<2,int>: public Interval<2,int> 
{

#define COUNTABLE int
#include "multi_array/CloseOpenInterval_COUNTABLE_2.h"
#undef COUNTABLE

#define UNCOUNTABLE int
#include "multi_array/CloseOpenInterval_UNCOUNTABLE_2.h"
#undef UNCOUNTABLE

};

template <>
class CloseOpenInterval<2,long>: public Interval<2,long> 
{

#define COUNTABLE long
#include "multi_array/CloseOpenInterval_COUNTABLE_2.h"
#undef COUNTABLE

#define UNCOUNTABLE long
#include "multi_array/CloseOpenInterval_UNCOUNTABLE_2.h"
#undef UNCOUNTABLE

};

template <>
class CloseOpenInterval<2,unsigned int>: public Interval<2,unsigned int> 
{

#define COUNTABLE unsigned int
#include "multi_array/CloseOpenInterval_COUNTABLE_2.h"
#undef COUNTABLE

#define UNCOUNTABLE unsigned int
#include "multi_array/CloseOpenInterval_UNCOUNTABLE_2.h"
#undef UNCOUNTABLE

};



