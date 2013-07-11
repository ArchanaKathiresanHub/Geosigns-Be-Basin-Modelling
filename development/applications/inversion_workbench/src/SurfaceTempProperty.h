#ifndef INVERSION_SURFACETEMP_PROPERTY_H
#define INVERSION_SURFACETEMP_PROPERTY_H

#include <string>
#include <vector>

#include "Property.h"
#include "ScalarRange.h"

class Scenario;

class SurfaceTempProperty : public Property
{
public:
   SurfaceTempProperty( double start, double end, double step)
    : m_range(start, end, step)
   {}

   virtual void reset();
   virtual void createParameter(Scenario & scenario) const;
   virtual void nextValue();
   virtual bool isPastEnd() const;

private:
   ScalarRange m_range;
};


#endif
