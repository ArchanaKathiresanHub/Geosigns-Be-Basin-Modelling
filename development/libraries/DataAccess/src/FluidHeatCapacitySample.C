#include <assert.h>

#include <iostream>
#include <sstream>
using namespace std;

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Interface/Formation.h"
#include "Interface/FluidHeatCapacitySample.h"
#include "Interface/ObjectFactory.h"
#include "Interface/ProjectHandle.h"
#include "Interface/FluidType.h"

using namespace DataAccess;
using namespace Interface;

FluidHeatCapacitySample::FluidHeatCapacitySample (ProjectHandle * projectHandle, Record * record) : DAObject (projectHandle, record)
{
   m_fluid = projectHandle->findFluid ( database::getFluidtype ( m_record ));
}


FluidHeatCapacitySample::~FluidHeatCapacitySample ()
{
}

const FluidType* FluidHeatCapacitySample::getFluid () const
{
   return m_fluid;
}

double FluidHeatCapacitySample::getTemperature () const
{
   return database::getTempIndex ( m_record );
}

double FluidHeatCapacitySample::getPressure () const
{
   return database::getPressure ( m_record );
}

double FluidHeatCapacitySample::getHeatCapacity () const
{
   return database::getHeatCapacity ( m_record );
}
