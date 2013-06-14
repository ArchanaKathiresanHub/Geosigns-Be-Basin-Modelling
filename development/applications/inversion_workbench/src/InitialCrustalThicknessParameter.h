#ifndef INVERSION_BASEMENTPARAMETER_H
#define INVERSION_BASEMENTPARAMETER_H

#include <string>

#include "parameter.h"

class Project;

class InitialCrustalThicknessParameter : public Parameter
{
public:
   InitialCrustalThicknessParameter(double value);
   virtual void print(std::ostream & output);
   virtual void changeParameter(Project & project);

private:
   double m_value;
};

#endif

