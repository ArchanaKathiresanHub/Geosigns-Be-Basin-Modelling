#include "InitialCrustalThicknessParameter.h"
#include "project.h"

#include <iostream>

InitialCrustalThicknessParameter::InitialCrustalThicknessParameter(double value) :
   m_value( value )
{
}

void InitialCrustalThicknessParameter::print(std::ostream & output)
{ 
   output << "InitialCrustalThickness = " << m_value ;
}

void InitialCrustalThicknessParameter::changeParameter(Project & project)
{
   project.setInitialCrustalThicknessProperty(m_value);
}


