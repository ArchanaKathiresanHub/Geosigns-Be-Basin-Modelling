#ifndef INVERSION_BASEMENT_PROPERTY_H
#define INVERSION_BASEMENT_PROPERTY_H

#include <string>
#include <vector>

class BasementProperty
{

public:
  BasementProperty(const std::string & name, double start, double end, double step)
    : m_name(name)
    , m_start(start)
    , m_end(end)
    , m_step(step) 
  {}

  const std::string & getName() const
  { return m_name; }

  double getStart() const
  { return m_start; }

  double getEnd() const
  { return m_end; }

  double getStep() const
  { return m_step; }


private:
  std::string m_name;
  double m_start, m_end, m_step;
};

#endif
