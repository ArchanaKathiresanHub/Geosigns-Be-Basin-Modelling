#ifndef _FUNCTIONS_FUNCTION_H_
#define _FUNCTIONS_FUNCTION_H_

namespace functions {

class Function
{
public:

  virtual double operator()(const double& value) const = 0;
  
  double apply(const double& value) const { return operator()(value); }

  virtual ~Function() {}
};

} // namespace functions

#endif
