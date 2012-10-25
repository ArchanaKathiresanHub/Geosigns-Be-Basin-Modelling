#ifndef _MULTI_ARRAY_RMSTRIDEDSTEPPEDCONSTCURSOR_H_
#define _MULTI_ARRAY_RMSTRIDEDSTEPPEDCONSTCURSOR_H_

#include "multi_array/ContiguousSteppedConstCursor.h"

#define RMSTRIDEDSTEPPEDCURSOR RMStridedSteppedConstCursor
#define CONST const
#define CONTIGUOUSSTEPPEDCURSOR ContiguousSteppedConstCursor
#include "multi_array/RMSTRIDEDSTEPPEDCURSOR_CONST.h"
#undef CONST
#undef CONTIGUOUSSTEPPEDCURSOR
#undef RMSTRIDEDSTEPPEDCURSOR

#endif
