#include "BasementProperty.h"
#include "BasementParameter.h"
#include "case.h"

void BasementProperty::reset()
{
   m_range.reset();
}

void BasementProperty::createParameter(Case & project) const
{
   project.addParameter( new BasementParameter( m_name, m_range.getValue()) );
}

void BasementProperty::nextValue()
{
   m_range.nextValue();
}

bool BasementProperty::isPastEnd() const
{
   return m_range.isPastEnd(); 
}

