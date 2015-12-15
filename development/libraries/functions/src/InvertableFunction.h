#ifndef _FUNCTIONS_INVERTABLEFUNCTION_H_
#define _FUNCTIONS_INVERTABLEFUNCTION_H_

#include "Function.h"

namespace functions {

class InvertableFunction: public Function
{
public:

   virtual double invert(const double& value) const = 0;
};

} // namespace functions

#endif
