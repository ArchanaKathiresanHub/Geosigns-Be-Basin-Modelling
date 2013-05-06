#ifndef INVERSION_CRUSTALTHINNINGPARAMETER_H
#define INVERSION_CRUSTALTHINNINGPARAMETER_H

#include <vector>
#include <utility>
#include "parameter.h"


class CrustalThinningParameter : public Parameter
{
public:
   typedef double Time; // in mA (million of years ago)
   typedef double Thickness; // in meters.
   typedef std::pair<Time, Thickness> ThicknessAtTime;

   CrustalThinningParameter(std::vector< ThicknessAtTime > & series);
   virtual void print();
   virtual void changeParameter(Project & project);

private:
   std::vector< ThicknessAtTime > m_series;
};

#endif

