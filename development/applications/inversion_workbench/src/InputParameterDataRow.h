#ifndef INVERSION_INPUT_PARAMETER_DATA_ROW_H
#define INVERSION_INPUT_PARAMETER_DATA_ROW_H

#include <string>
#include <vector>
#include <cmath>

class InputParameterDataRow
{
public:
   InputParameterDataRow(const std::string & name, double start, double end, double step)
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

   int calculateNumberOfCases() const
   { 
      double nb;
      if ( m_end != m_start )
      {
         nb = (std::abs(m_end-m_start)) / abs(m_step) + 1;
      }
      else if ( m_end == m_start )
      {
         nb = 1;
      }
      return int(nb);
   }


private:
   std::string m_name;
   double m_start, m_end, m_step;
};

#endif
