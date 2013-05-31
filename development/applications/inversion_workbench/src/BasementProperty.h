#ifndef INVERSION_BASEMENT_PROPERTY_H
#define INVERSION_BASEMENT_PROPERTY_H

#include <string>
#include <vector>

#include "Property.h"
#include "ScalarRange.h"

class Scenario;

class BasementProperty : public Property
{
public:
   BasementProperty(const std::string & name, double start, double end, double step)
    : m_name(name)
    , m_range(start, end, step)
   {}

   virtual void reset();
   virtual void createParameter(Scenario & scenario) const;
   virtual void nextValue();
   virtual bool isPastEnd() const;

private:
   std::string m_name;
   ScalarRange m_range;
};


#endif
