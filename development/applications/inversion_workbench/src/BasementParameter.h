#ifndef INVERSION_BASEMENTPARAMETER_H
#define INVERSION_BASEMENTPARAMETER_H

#include <string>

#include "parameter.h"

class Project;

class BasementParameter : public Parameter
{
public:
   BasementParameter( const std::string & name, double value );

   virtual ~BasementParameter() {;}

   virtual void print( std::ostream & output );
   virtual void changeParameter( Project & project );

   /// All basement parameters are continious now
   virtual bool isContinuous() const { return true; }

   /// Return parameter value as vector of doubles
   virtual std::vector<double> toDblVector() const;

   /// Set value for parameter from array of doubles
   virtual void fromDblVector( const std::vector<double> & prms );

private:
   std::string m_name;
   double m_value;
};

#endif

