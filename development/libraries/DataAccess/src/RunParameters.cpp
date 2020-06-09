#include <assert.h>

#include <iostream>
#include <sstream>
using namespace std;

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "RunParameters.h"
#include "ProjectHandle.h"

#include "errorhandling.h"
#include "NumericFunctions.h"

using namespace DataAccess;
using namespace Interface;


RunParameters::RunParameters (ProjectHandle& projectHandle, Record * record) : DAObject (projectHandle, record)
{

   const string& optimisationLevelStr = getOptimisationLevelStr ();

   // The string will always have the format "Level N ...". Where N is the optimisation level
   // Optimisation Level is a string, eg "Level 2", the character at the 6th
   // position is the optimisation-level.
   if ( NumericFunctions::inRange<char>( optimisationLevelStr [ 6 ], '1', '5' )) {
      m_optimisationLevel = int ( optimisationLevelStr [ 6 ] ) - int ( '0' );
   } else {
      m_optimisationLevel = 4;
   }

}

RunParameters::~RunParameters ()
{
}

const std::string& RunParameters::getVreAlgorithm () const {
   return database::getVreAlgorithm ( m_record );
}

SeismicVelocityModel RunParameters::getSeismicVelocityAlgorithm() const {
   const std::string& seismicVelocityModelStr = database::getVelAlgorithm( m_record );
   if (seismicVelocityModelStr == "Gardner\'s Velocity-Density") {
      return GARDNERS_VELOCITY_ALGORITHM;
   }
   else if (seismicVelocityModelStr == "Wyllie\'s Time-Average") {
      return WYLLIES_VELOCITY_ALGORITHM;
   }
   else if (seismicVelocityModelStr == "Kennan\'s Velocity-Porosity") {
      return KENNANS_VELOCITY_ALGORITHM;
   }
   else {
      std::cout << "Basin_Error: Error in seismic velocity model " << seismicVelocityModelStr << ". Using Gardner seismic velocity as the default value." << endl;
      return GARDNERS_VELOCITY_ALGORITHM;
   }
}

const std::string& RunParameters::getChemicalCompactionAlgorithm () const{
   return database::getChemicalCompactionAlgorithm ( m_record );
}

double RunParameters::getPrefReconstep () const {
   double prefReconstep(database::getPrefReconstep ( m_record ));
   if ( prefReconstep <= 0 ) throw RecordException( "Unvalid value of PrefReconstep: %; Cauldron cannot run back in time",prefReconstep);
   return prefReconstep;
}

double RunParameters::getOptimalSourceRockTempDiff () const {
   return database::getOptimalSourceRockTempDiff ( m_record );
}

double RunParameters::getOptimalTotalTempDiff () const {
   return database::getOptimalTotalTempDiff ( m_record );
}

double RunParameters::getOptimalTotalPresDiff () const {
   return database::getOptimalTotalPresDiff ( m_record );
}

double RunParameters::getMaxTimeStepIncreaseFactor () const {
   return database::getMaxTimeStepIncreaseFactor ( m_record );
}

double RunParameters::getMinTimeStepDecreaseFactor () const {
   return database::getMinTimeStepDecreaseFactor ( m_record );
}

double RunParameters::getBrickHeightSediment () const {
   return database::getBrickHeightSediment ( m_record );
}

double RunParameters::getBrickHeightCrust () const {
   return database::getBrickHeightCrust ( m_record );
}

double RunParameters::getBrickHeightMantle () const {
   return database::getBrickHeightMantle ( m_record );
}

int RunParameters::getMaxNumberOfRunOverpressure () const {
   return database::getMaxNumberOfRunOverpressure ( m_record );
}

double RunParameters::getTemperatureGradient  () const {
   return database::getTemperature_Gradient ( m_record );
}

int RunParameters::getOptimisationLevel  () const {
   return m_optimisationLevel;
}

const std::string& RunParameters::getOptimisationLevelStr  () const {
   return database::getOptimisation_Level ( m_record );
}

bool RunParameters::getAdvancedMigration () const {
   return database::getAdvancedMigration ( m_record ) != 0;
}

bool RunParameters::getHydrodynamicCapillaryPressure () const {
   return database::getHydrodynamicCapillaryPressure ( m_record ) != 0;
}

bool RunParameters::getReservoirDetection () const {
   return database::getReservoirDetection ( m_record ) != 0;
}

bool RunParameters::getPaleoSeeps () const {
   return database::getPaleoSeeps ( m_record ) != 0;
}

bool RunParameters::getLegacy () const {
   return database::getLegacy ( m_record ) != 0;
}

double RunParameters::getMinOilColumnHeight () const {
   return database::getMinOilColumnHeight ( m_record );
}

double RunParameters::getMinGasColumnHeight () const {
   return database::getMinGasColumnHeight ( m_record );
}

bool RunParameters::getBlockingInd () const {
   return database::getBlockingInd ( m_record ) != 0;
}

double RunParameters::getBlockingPermeability  () const {
   return database::getBlockingPermeability ( m_record );
}

double RunParameters::getBlockingPorosity  () const {
   return database::getBlockingPorosity ( m_record );
}

const std::string& RunParameters::getFracturePressureFunction () const {
   return database::getFracturePressureFunction ( m_record );
}

const std::string& RunParameters::getFractureType () const {
   return database::getFractureType ( m_record );
}

int RunParameters::getFractureModel () const {
   return database::getFractureModel ( m_record );
}


bool RunParameters::getChemicalCompaction () const {
   return database::getChemicalCompaction ( m_record ) == 1;
}

bool RunParameters::getConvectiveTerm () const {
   return database::getConvectiveTerm ( m_record ) == 1;
}

bool RunParameters::getNonGeometricLoop () const {
   return database::getNonGeometricLoop ( m_record ) == 1;
}

bool RunParameters::getAllochthonousModelling () const {
   return database::getAllochthonousModelling ( m_record ) == 1;
}

bool RunParameters::getApplyOtgcToDarcy () const {
   return database::getApplyOtgcToDarcy ( m_record ) == 1;
}


bool RunParameters::useBurialRateTimeStepping () const {
   return database::getTempDiffBasedStepping ( m_record ) == 0;
}

double RunParameters::getSegmentFractionToBury () const {
   double stepsPerSegment(database::getStepsPerSegment ( m_record ));
   if ( stepsPerSegment <= 0 ) throw RecordException( "Unvalid value of StepsPerSegment: %; Cauldron cannot run back in time",stepsPerSegment);
   return stepsPerSegment;
}

double RunParameters::getDarcyMaxTimeStep () const {
   return database::getDarcyMaxTimeStep ( m_record );
}

void RunParameters::printOn (ostream & ostr) const {
   string str;
   asString (str);
   ostr << str;
}

void RunParameters::asString ( std::string & str ) const {

   ostringstream buf;

   buf << "RunParameters:";
   buf << endl;

   str = buf.str ();
}
