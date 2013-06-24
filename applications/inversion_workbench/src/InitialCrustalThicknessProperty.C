#include "InitialCrustalThicknessProperty.h"
#include "InitialCrustalThicknessParameter.h"
#include "Scenario.h"

void InitialCrustalThicknessProperty::reset()
{
   m_range.reset();
}

void InitialCrustalThicknessProperty::createParameter(Scenario & scenario) const
{
   scenario.addParameter( new InitialCrustalThicknessParameter( m_range.getValue()) );
}

void InitialCrustalThicknessProperty::nextValue()
{
   m_range.nextValue();
}

bool InitialCrustalThicknessProperty::isPastEnd() const
{
   return m_range.isPastEnd(); 
}

