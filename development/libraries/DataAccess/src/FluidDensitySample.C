#include <assert.h>

#include <iostream>
#include <sstream>
using namespace std;

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Formation.h"
#include "FluidDensitySample.h"
#include "ObjectFactory.h"
#include "ProjectHandle.h"
#include "FluidType.h"

using namespace DataAccess;
using namespace Interface;

FluidDensitySample::FluidDensitySample (ProjectHandle& projectHandle, Record * record) : DAObject (projectHandle, record)
{
   m_fluid = projectHandle.findFluid ( database::getFluidtype ( m_record ));
}

FluidDensitySample::~FluidDensitySample ()
{
}

const FluidType* FluidDensitySample::getFluid () const
{
   return m_fluid;
}

double FluidDensitySample::getTemperature () const
{
   return database::getTempIndex ( m_record );
}

double FluidDensitySample::getPressure () const
{
   return database::getPressure ( m_record );
}

double FluidDensitySample::getDensity () const
{
   return database::getDensity ( m_record );
}
