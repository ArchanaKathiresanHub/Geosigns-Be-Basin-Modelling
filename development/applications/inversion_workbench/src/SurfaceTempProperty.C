#include "SurfaceTempProperty.h"
#include "SurfaceTempParameter.h"
#include "Scenario.h"

void SurfaceTempProperty :: reset()
{
   m_range.reset();
}

void SurfaceTempProperty :: createParameter(Scenario & scenario) const
{
   scenario.addParameter( new SurfaceTempParameter( m_range.getValue() ) );
}

void SurfaceTempProperty :: nextValue()
{
   m_range.nextValue();
}

bool SurfaceTempProperty :: isPastEnd() const
{
   return m_range.isPastEnd();
}
