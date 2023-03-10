#include <assert.h>

#include <iostream>
#include <sstream>
using namespace std;

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Formation.h"
#include "FluidHeatCapacitySample.h"
#include "ObjectFactory.h"
#include "ProjectHandle.h"
#include "FluidType.h"

using namespace DataAccess;
using namespace Interface;

FluidHeatCapacitySample::FluidHeatCapacitySample (ProjectHandle& projectHandle, Record * record) : DAObject (projectHandle, record)
{
   m_fluid = projectHandle.findFluid ( database::getFluidtype ( m_record ));
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
