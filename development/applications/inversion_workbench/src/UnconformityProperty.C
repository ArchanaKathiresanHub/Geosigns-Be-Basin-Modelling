#include "UnconformityProperty.h"
#include "UnconformityParameter.h"
#include "Scenario.h"

void UnconformityProperty::createParameter(Scenario & scenario) const
{
   scenario.addParameter( new UnconformityParameter(m_depoFormationName, m_name, m_range.getValue()) );
}

void UnconformityProperty::reset()
{
   m_range.reset();
}

void UnconformityProperty::nextValue()
{
   m_range.nextValue();
}

void UnconformityProperty::lastValue()
{
   m_range.lastValue();
}


bool UnconformityProperty::isPastEnd() const
{
   return m_range.isPastEnd(); 
}

