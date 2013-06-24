#ifndef INVERSION_CRUSTALTHINNINGPARAMETER_H
#define INVERSION_CRUSTALTHINNINGPARAMETER_H

#include "parameter.h"


class CrustalThinningParameter : public Parameter
{
public:
   CrustalThinningParameter(double startTime, double duration, double ratio);
   virtual void print(std::ostream & output );
   virtual void changeParameter(Project & project);

private:
   double m_startTime; // in Ma
   double m_duration; // in million years
   double m_ratio; // thinning ratio
};

#endif

