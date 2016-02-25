#include   "filterwizard.h"

#include <string>
using std::string;

Filterwizard::Filterwizard(TimeFilter* tf){
  
  SetDerivedProperties = false;
   
  bBulkDensities = false;
  bVelocity      = false;
  bSonic         = false;
  bReflectivity  = false;
  bIsoStaticMass = false;

  bIsoStaticDeflection = false;
  bIsoStaticWaterBottom = false;

  timefilter = tf;

}

Filterwizard::~Filterwizard(){};

bool Filterwizard::IsBiomarkersCalculationNeeded()
{
   bool bCalculationNeeded = false;

   bCalculationNeeded = timefilter->PropertyOutputOption[BIOMARKERS];

   return (bCalculationNeeded);

}


bool Filterwizard::IsBulkThCondCalculationNeeded()
{

   bool bCalculationNeeded = false;

   bCalculationNeeded = 
     ( timefilter->PropertyOutputOption[THCONDVEC] ||
       IsDiffusivityCalculationNeeded() ||
       IsHeatFlow_CalculationNeeded() );

   return (bCalculationNeeded);

}

bool Filterwizard::IsBulkPermeabilityCalculationNeeded(){

   bool bCalculationNeeded = false;

   bCalculationNeeded = timefilter->PropertyOutputOption[PERMEABILITYVEC];

   return (bCalculationNeeded);

}

bool Filterwizard::IsDecompactionNeeded()
{

   bool bCalculationNeeded = false;

   bCalculationNeeded = timefilter->IsSomethingSelected();

   return (bCalculationNeeded);

}


bool Filterwizard::IsDiffusivityCalculationNeeded()
{

   bool bCalculationNeeded = false;

   bCalculationNeeded = timefilter->PropertyOutputOption[DIFFUSIVITYVEC];

   return (bCalculationNeeded);

}
bool Filterwizard::IsFissionTrackCalculationNeeded()
{

   bool bCalculationNeeded = false;

   return (bCalculationNeeded);

}


bool Filterwizard::IsHeatFlowCalculationNeeded()
{

   bool bCalculationNeeded = false;

   bCalculationNeeded = timefilter->PropertyOutputOption[HEAT_FLOW];

   return (bCalculationNeeded);

}


bool Filterwizard::IsHeatFlow_CalculationNeeded()
{

   bool bCalculationNeeded = false;

   bCalculationNeeded = timefilter->PropertyOutputOption[HEAT_FLOW_];

   return (bCalculationNeeded);

}


bool Filterwizard::IsMassFluxCalculationNeeded()
{

   bool bCalculationNeeded = false;

   bCalculationNeeded = timefilter->PropertyOutputOption[MASSFLUX];

   return (bCalculationNeeded);

}


bool Filterwizard::IsMassFlux_CalculationNeeded()
{

   bool bCalculationNeeded = false;

   bCalculationNeeded = timefilter->PropertyOutputOption[MASSFLUX_];

   return (bCalculationNeeded);

}


bool Filterwizard::IsPressureCalculationNeeded()
{

   bool bCalculationNeeded = false;

   bCalculationNeeded =
         ( timefilter->PropertyOutputOption[PRESSURE] ||
	   timefilter->PropertyOutputOption[HYDROSTATICPRESSURE] ||
	   timefilter->PropertyOutputOption[OVERPRESSURE] ||
	   IsMassFluxCalculationNeeded() ||
	   IsMassFlux_CalculationNeeded() );

   return (bCalculationNeeded);

}


bool Filterwizard::IsSmectiteIlliteCalculationNeeded()
{

   bool bCalculationNeeded = false;

   bCalculationNeeded = timefilter->PropertyOutputOption[ILLITEFRACTION];

   return (bCalculationNeeded);

}


bool Filterwizard::IsTemperatureCalculationNeeded()
{

   bool bCalculationNeeded = false;

   bCalculationNeeded = 
     ( timefilter->PropertyOutputOption[TEMPERATURE] ||
       IsHeatFlowCalculationNeeded() ||
       IsHeatFlow_CalculationNeeded() ||
       IsVreCalculationNeeded() ||
       IsBiomarkersCalculationNeeded() ||
       IsSmectiteIlliteCalculationNeeded() ||
       IsFissionTrackCalculationNeeded() ||
       IsBulkThCondCalculationNeeded());

   return (bCalculationNeeded);

}


bool Filterwizard::IsVreCalculationNeeded()
{

   bool bCalculationNeeded = true;

   return (bCalculationNeeded);

}


bool Filterwizard::IsXmigCalculationNeeded()
{
   bool bCalculationNeeded = false;

   return (bCalculationNeeded);

}

bool Filterwizard::InitDerivedCalculationsNeeded()
{

  bool bCalculationNeeded = false;
  
  if (timefilter->PropertyOutputOption[REFLECTIVITYVEC])
    {
      bReflectivity = true;
      bVelocity = true;
      bBulkDensities = true;
      bCalculationNeeded = true;
    }
  
  if (timefilter->PropertyOutputOption[SONICVEC])
    {
      bSonic = true;
      bVelocity = true;
      bBulkDensities = true;
      bCalculationNeeded = true;
    }
  
  if (timefilter->PropertyOutputOption[VELOCITYVEC])
    {
      bVelocity = true;
      bBulkDensities = true;
      bCalculationNeeded = true;
    }
  
  if (timefilter->PropertyOutputOption[BULKDENSITYVEC])
    {
      bBulkDensities = true;
      bCalculationNeeded = true;
    }
  
  if (timefilter->PropertyOutputOption[ISOSTATICMASS])
    {
      bIsoStaticMass = true;
      bBulkDensities = true;
      bCalculationNeeded = true;
    }
  if (timefilter->PropertyOutputOption[ISOSTATICDEFLECTION])
    {
      bIsoStaticDeflection = true;
      bIsoStaticMass = true;
      bBulkDensities = true;
      bCalculationNeeded = true;
    }
  if (timefilter->PropertyOutputOption[ISOSTATICWB])
    {
      bIsoStaticWaterBottom = true;
      bIsoStaticDeflection = true;
      bIsoStaticMass = true;
      bBulkDensities = true;
      bCalculationNeeded = true;
    }

  if (bCalculationNeeded) SetDerivedProperties = true;

  return (bCalculationNeeded);
  
}

bool Filterwizard::fluidVelocityIsNeeded () const {
  return timefilter->PropertyOutputOption[FLUID_VELOCITY] > NOOUTPUT;
}

bool Filterwizard::thicknessCalculationNeeded () const {
  return timefilter->PropertyOutputOption[THICKNESS] > NOOUTPUT;
}
