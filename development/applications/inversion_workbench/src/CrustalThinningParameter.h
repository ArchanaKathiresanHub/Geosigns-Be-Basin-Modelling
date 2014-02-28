#ifndef INVERSION_CRUSTALTHINNINGPARAMETER_H
#define INVERSION_CRUSTALTHINNINGPARAMETER_H

#include "parameter.h"


class CrustalThinningParameter : public Parameter
{
public:
   CrustalThinningParameter(double startTime, double duration, double ratio);
   virtual void print(std::ostream & output );
   virtual void changeParameter(Project & project);

   /// All crustal thinning parameters parameters are continious now
   virtual bool isContinuous() const { return true; }

   /// Return parameter value as vector of doubles
   virtual std::vector<double> toDblVector() const;

   /// Set value for parameter from array of doubles
   virtual void fromDblVector( const std::vector<double> & prms );


private:
   double m_startTime; // in Ma
   double m_duration; // in million years
   double m_ratio; // thinning ratio
};

#endif

