#ifndef INVERSION_BASEMENTPARAMETER_H
#define INVERSION_BASEMENTPARAMETER_H

#include <string>

#include "parameter.h"

class Project;

class BasementParameter : public Parameter
{
public:
   BasementParameter(const std::string & name, double value);
   virtual void print(std::ostream & output);
   virtual void changeParameter(Project & project);

private:
   std::string m_name;
   double m_value;
};

#endif

