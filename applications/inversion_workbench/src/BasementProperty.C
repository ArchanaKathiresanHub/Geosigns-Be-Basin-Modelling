#include "BasementProperty.h"
#include "BasementParameter.h"
#include "Scenario.h"

void BasementProperty::reset()
{
   m_range.reset();
}

void BasementProperty::createParameter(Scenario & scenario) const
{
   scenario.addParameter( new BasementParameter( m_name, m_range.getValue()) );
}

void BasementProperty::nextValue()
{
   m_range.nextValue();
}

bool BasementProperty::isPastEnd() const
{
   return m_range.isPastEnd(); 
}

