#if (defined (_WIN32) || defined (_WIN64))
#define NOMINMAX
#endif

#include "DiffusionLeakageParameters.h"
#include "cauldronschemafuncs.h"

#include <math.h>

#include <assert.h>
#include <limits>

using namespace database;
using namespace DataAccess;
using std::numeric_limits;

namespace DataAccess { namespace Interface {

DiffusionLeakageParameters::DiffusionLeakageParameters(ProjectHandle& projecthandle, database::Record* record):
   DAObject(projecthandle, record)
{}

DiffusionLeakageParameters::~DiffusionLeakageParameters()
{}

double DiffusionLeakageParameters::salinity() const
{
  return database::getDiffSalinity(m_record);
}

OverburdenModel DiffusionLeakageParameters::overburdenModel() const
{
  return database::getSealThicknessModel(m_record) == "FullOverburden" ?
    FullOverburden :
    LimitedOverburden;
}

double DiffusionLeakageParameters::maximumSealThickness() const
{
   return (overburdenModel() == FullOverburden) ?
      numeric_limits<double>::max() : database::getMaximumSealThickness(m_record);
}

TransientModel DiffusionLeakageParameters::transientModel() const
{
  return database::getDiffusionModel(m_record) == "Transient" ?
    Transient :
    SteadyState;
}

std::vector<double> DiffusionLeakageParameters::diffusionConsts() const
{
  std::vector<double> diffusionConsts;
  diffusionConsts.push_back( database::getC1_DiffFactor(m_record) );
  diffusionConsts.push_back( database::getC2_DiffFactor(m_record) );
  diffusionConsts.push_back( database::getC3_DiffFactor(m_record) );
  diffusionConsts.push_back( database::getC4_DiffFactor(m_record) );
  diffusionConsts.push_back( database::getC5_DiffFactor(m_record) );

  return diffusionConsts;
}

double DiffusionLeakageParameters::activationEnergy() const
{
  return database::getActEnergy(m_record);
}

std::vector<double> DiffusionLeakageParameters::concentrationConsts() const
{
  std::vector<double> concentrationConsts;
  concentrationConsts.push_back( database::getC1_ConcFactor(m_record) );
  concentrationConsts.push_back( database::getC2_ConcFactor(m_record) );
  concentrationConsts.push_back( database::getC3_ConcFactor(m_record) );
  concentrationConsts.push_back( database::getC4_ConcFactor(m_record) );
  concentrationConsts.push_back( database::getC5_ConcFactor(m_record) );

  return concentrationConsts;
}

} } // namespace DataAccess::Interface
