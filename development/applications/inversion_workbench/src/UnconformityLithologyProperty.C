#include "UnconformityLithologyProperty.h"
#include "UnconformityLithologyParameter.h"
#include "Scenario.h"

void UnconformityLithologyProperty::createParameter(Scenario & scenario) const
{
   scenario.addParameter( new UnconformityLithologyParameter( m_depoFormationName,
	    m_lithology1, m_percentage1,
	    m_lithology2, m_percentage2,
	    m_lithology3, m_percentage3));
}

void UnconformityLithologyProperty::reset()
{

   m_pastEnd = false;
}

void UnconformityLithologyProperty::nextValue()
{
   m_pastEnd = true;
}

bool UnconformityLithologyProperty::isPastEnd() const
{
   return m_pastEnd;
}

