#ifndef INVERSION_CRUSTALTHINNINGPROPERTY_H
#define INVERSION_CRUSTALTHINNINGPROPERTY_H

#include <vector>

#include "Property.h"
#include "ScalarRange.h"
#include "MultiDimRange.h"

class Scenario;

class CrustalThinningProperty : public Property
{
public:
   CrustalThinningProperty(ScalarRange t0, ScalarRange dt, ScalarRange dz, ScalarRange ratio);

   virtual void reset();
   virtual void createParameter(Scenario & scenario) const ;
   virtual void nextValue() ;
   virtual bool isPastEnd() const ;

private:
   MultiDimRange m_range;
};


#endif
