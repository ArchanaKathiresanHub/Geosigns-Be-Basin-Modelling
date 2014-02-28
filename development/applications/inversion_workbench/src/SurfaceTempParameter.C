#include "SurfaceTempParameter.h"
#include "project.h"

#include <iostream>
#include <cassert>

SurfaceTempParameter :: SurfaceTempParameter( double temperature )
   : m_temperature(temperature)
{
}

void SurfaceTempParameter :: print( std::ostream &  output )
{
   output << "Surface temperature = " << m_temperature;
}

void SurfaceTempParameter :: changeParameter( Project & project)
{
   project.setSurfaceTemperature(m_temperature);
}

std::vector<double> SurfaceTempParameter::toDblVector() const 
{
   return std::vector<double>( 1, m_temperature );
}

void SurfaceTempParameter::fromDblVector( const std::vector<double> & prms )
{
   assert( prms.size() == 1 );
   m_temperature = prms[0];
}
