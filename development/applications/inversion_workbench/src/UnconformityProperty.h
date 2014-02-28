#ifndef INVERSION_UNCONFORMITY_PROPERTY_H
#define INVERSION_UNCONFORMITY_PROPERTY_H

#include <string>
#include <vector>

#include "Property.h"
#include "ScalarRange.h"

class Scenario;

class UnconformityProperty : public Property
{
public:
   UnconformityProperty(const std::string & depoFormationName, const std::string & name, double start, double end, double step)
    : m_depoFormationName(depoFormationName),
    m_name(name),
    m_range(start, end, step)
   {}

   virtual void reset();
   virtual void createParameter(Scenario & scenario) const;
   virtual void nextValue();
   virtual void lastValue();
   virtual bool isPastEnd() const;

private:
   std::string m_depoFormationName;
   std::string m_name;
   ScalarRange m_range;
};


#endif
