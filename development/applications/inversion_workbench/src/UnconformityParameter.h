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

   /// Unconformity parameter continious
   virtual bool isContinuous() const { return true; }

   /// Return parameter value as vector of doubles
   virtual std::vector<double> toDblVector() const;

   /// Set value for parameter from array of doubles
   virtual void fromDblVector( const std::vector<double> & prms );

private:
   std::string m_depoFormationName;
   std::string m_name;
   double m_value;
};

#endif

