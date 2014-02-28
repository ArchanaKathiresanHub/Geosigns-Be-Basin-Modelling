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

   /// Initial crustal thickness is a continious parameter
   virtual bool isContinuous() const { return true; }

   /// Return parameter value as vector of doubles
   virtual std::vector<double> toDblVector() const;

   /// Set value for parameter from array of doubles
   virtual void fromDblVector( const std::vector<double> & prms );


private:
   double m_value;
};

#endif

