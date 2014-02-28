#include "InitialCrustalThicknessParameter.h"
#include "project.h"

#include <iostream>
#include <cassert>

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

std::vector<double> InitialCrustalThicknessParameter::toDblVector() const 
{
   return std::vector<double>( 1, m_value );
}

void InitialCrustalThicknessParameter::fromDblVector( const std::vector<double> & prms )
{
   assert( prms.size() == 1 );
   m_value = prms[0];
}
