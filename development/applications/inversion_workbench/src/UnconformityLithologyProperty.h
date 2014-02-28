#ifndef INVERSION_UNCONFORMITY_LITHOLOGY_PROPERTY_H
#define INVERSION_UNCONFORMITY_LITHOLOGY_PROPERTY_H

#include <string>
#include <vector>

#include "Property.h"

class Scenario;

class UnconformityLithologyProperty : public Property
{
public:
   UnconformityLithologyProperty(const std::string & depoFormationName,
	 const std::string lithology1, const double percentage1,
	 const std::string lithology2, const double percentage2,
	 const std::string lithology3, const double percentage3) :

    m_depoFormationName(depoFormationName),
    m_lithology1 (lithology1), m_percentage1 (percentage1),
    m_lithology2 (lithology2), m_percentage2 (percentage2),
    m_lithology3 (lithology3), m_percentage3 (percentage3),
    m_pastEnd (true)
   {}

   virtual void reset();
   virtual void createParameter(Scenario & scenario) const;
   virtual void nextValue();
   virtual void lastValue();
   virtual bool isPastEnd() const;

private:
   std::string m_depoFormationName;

   std::string m_lithology1;
   const double m_percentage1;
   std::string m_lithology2;
   const double m_percentage2;
   std::string m_lithology3;
   const double m_percentage3;

   bool m_pastEnd;
};


#endif
