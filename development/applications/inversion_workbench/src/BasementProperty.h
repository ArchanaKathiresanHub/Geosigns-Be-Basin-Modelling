#ifndef INVERSION_BASEMENT_PROPERTY_H
#define INVERSION_BASEMENT_PROPERTY_H

#include <string>
#include <vector>

#include "Property.h"

class Case;

class BasementProperty : public Property
{
public:
   BasementProperty(const std::string & name, double start, double end, double step)
    : Property(name, start, end, step)
   {}

   virtual void createParameter(Case & project, double value);
};


#endif
