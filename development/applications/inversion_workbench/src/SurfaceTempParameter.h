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

private:
   double m_temperature;
};

#endif
