#ifndef INVERSION_UNCONFORMITY_PARAMETER_H
#define INVERSION_UNCONFORMITY_PARAMETER_H

#include <string>

#include "parameter.h"

class Project;

class UnconformityParameter : public Parameter
{
public:
   UnconformityParameter( const std::string & depoFormationName, const std::string & name, double value);
   virtual void print(std::ostream & output);
   virtual void changeParameter(Project & project);

private:
   std::string m_depoFormationName;
   std::string m_name;
   double m_value;
};

#endif

