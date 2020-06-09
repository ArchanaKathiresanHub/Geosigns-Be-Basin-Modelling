#include <assert.h>

#include <iostream>
#include <sstream>
using namespace std;

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Formation.h"
#include "FluidThermalConductivitySample.h"
#include "ObjectFactory.h"
#include "ProjectHandle.h"
#include "FluidType.h"

using namespace DataAccess;
using namespace Interface;

FluidThermalConductivitySample::FluidThermalConductivitySample (ProjectHandle& projectHandle, Record * record) : DAObject (projectHandle, record)
{
   m_fluid = projectHandle.findFluid ( database::getFluidtype ( m_record ));
}

FluidThermalConductivitySample::~FluidThermalConductivitySample ()
{
}

const FluidType* FluidThermalConductivitySample::getFluid () const
{
   return m_fluid;
}

double FluidThermalConductivitySample::getTemperature () const
{
   return database::getTempIndex ( m_record );
}

double FluidThermalConductivitySample::getPressure () const
{
   return database::getPressure ( m_record );
}

double FluidThermalConductivitySample::getThermalConductivity () const
{
   return database::getThCond ( m_record );
}
