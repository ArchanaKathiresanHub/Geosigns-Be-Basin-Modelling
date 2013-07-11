#include "SurfaceTempParameter.h"
#include "project.h"

#include <iostream>

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
