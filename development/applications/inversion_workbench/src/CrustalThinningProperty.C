#include <iostream>
#include <vector>
#include "CrustalThinningProperty.h"
#include "CrustalThinningParameter.h"
#include "Scenario.h"

#include <cmath>


namespace 
{
   std::vector< ScalarRange > operator+( ScalarRange a, ScalarRange b)
   {
      std::vector<ScalarRange > x;
      x.push_back(a);
      x.push_back(b);
      return x;
   }

   std::vector< ScalarRange > operator+( const std::vector<ScalarRange> & xs, ScalarRange b)
   {
      std::vector<ScalarRange> ys(xs);
      ys.push_back(b);
      return ys;
   }
}

CrustalThinningProperty::CrustalThinningProperty( ScalarRange t0, ScalarRange dt, ScalarRange dz, ScalarRange ratio)
    : m_range( t0 + dt + dz + ratio ) 
{
}

void CrustalThinningProperty::reset()
{
   m_range.reset();
}


void CrustalThinningProperty::nextValue()
{
   m_range.nextValue();
}

bool CrustalThinningProperty::isPastEnd() const
{
   return m_range.isPastEnd();
}

void CrustalThinningProperty::createParameter(Scenario & scenario) const
{
   std::vector< double > value;
   m_range.getValue(value);

   assert( value.size() == 4);
   scenario.addParameter( new CrustalThinningParameter( value[0], value[1], value[2], value[3] ) );
}
