#include "FluidType.h"
#include "cauldronschemafuncs.h"

#include <assert.h>

using namespace database;
using namespace DataAccess;
using namespace Interface;


FluidType::FluidType(ProjectHandle& projecthandle, database::Record* record):
   DAObject(projecthandle, record)
{

  if ( database::getDensModel(m_record) == "Calculated" ) {
     m_fluidDensityModel = CBMGenerics::waterDensity::Calculated;
  } else {
     m_fluidDensityModel = CBMGenerics::waterDensity::Constant;
  }

  if ( database::getSeisVelocityModel ( m_record ) == "Calculated" ) {
     m_seismicVelocityCalculationModel = CALCULATED_MODEL;
  } else {
     // The only other selectable option is constant.
     m_seismicVelocityCalculationModel = CONSTANT_MODEL;
  }

  // The default viscosity model.
  m_viscosityModel = BATTZLE_WANG_VISCOSITY;
}

FluidType::~FluidType()
{
}


const std::string& FluidType::getName () const {
   return database::getFluidtype ( m_record );
}

CBMGenerics::waterDensity::FluidDensityModel FluidType::fluidDensityModel() const
{
   return m_fluidDensityModel;
}

double FluidType::density() const
{
  return database::getDensity(m_record);
}

double FluidType::salinity() const
{
  return database::getSalinity(m_record);
}


void FluidType::setViscosityModel ( const ViscosityModel model ) {
   m_viscosityModel = model;
}


Interface::ViscosityModel FluidType::getViscosityModel () const {
   return m_viscosityModel;
}

Interface::CalculationModel FluidType::getSeismicVelocityCalculationModel () const {
   return m_seismicVelocityCalculationModel;
}

double FluidType::getConstantSeismicVelocity () const {
   return database::getSeisVelocity ( m_record );
}

const std::string& FluidType::getHeatCapacityFluidName () const {
   return database::getHeatCaptype ( m_record );
}

const FluidType* FluidType::getHeatCapacityFluidType () const {
   return getProjectHandle().findFluid ( getHeatCapacityFluidName ());
}

const std::string& FluidType::getThermalConductivityFluidName () const {
   return database::getThermCondtype ( m_record );
}

const FluidType* FluidType::getThermalConductivityFluidType () const {
   return getProjectHandle().findFluid ( getThermalConductivityFluidName ());
}

FluidHeatCapacitySampleList* FluidType::getFluidHeatCapacitySamples () const {
   return getProjectHandle().getFluidHeatCapacitySampleList ( this );
}

FluidThermalConductivitySampleList* FluidType::getFluidThermalConductivitySamples () const {
   return getProjectHandle().getFluidThermalConductivitySampleList ( this );
}
