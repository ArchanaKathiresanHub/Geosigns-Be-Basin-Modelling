#ifndef INVERSION_PROPERTY_H
#define INVERSION_PROPERTY_H

#include <string>

class Case;

class Property
{
public:
   Property(const std::string & name, double start, double end, double step)
      : m_name(name)
      , m_start(start)
      , m_end(end)
      , m_step(step)
   {};

   virtual ~Property() {}

   virtual void createParameter(Case & project, double value)=0;

   const std::string & getName() const
   { return m_name; }

   double getStart() const
   { return m_start; }

   double getEnd() const
   { return m_end; }

   double getStep() const
   { return m_step; }

protected:
   std::string m_name;
   double m_start, m_end, m_step;
};


#endif;
