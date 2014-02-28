#ifndef INVERSION_SURFACETEMPPARAMETER_H
#define INVERSION_SURFACETEMPPARAMETER_H

#include "parameter.h"

class Project;

class SurfaceTempParameter : public Parameter
{
public:
   SurfaceTempParameter( double temperature );
   virtual void print(std::ostream & output);
   virtual void changeParameter(Project & project);

   /// parameter is continious 
   virtual bool isContinuous() const { return true; }

   /// Return parameter value as vector of doubles
   virtual std::vector<double> toDblVector() const;

   /// Set value for parameter from array of doubles
   virtual void fromDblVector( const std::vector<double> & prms );

private:
   double m_temperature;
};

#endif
