#include "BasementProperty.h"
#include "BasementParameter.h"
#include "case.h"

void BasementProperty::createParameter(Case & project, double value)
{
  project.addParameter( new BasementParameter( this->getName(), value) );
}

