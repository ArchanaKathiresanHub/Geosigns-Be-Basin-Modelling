#include <assert.h>

#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      #include <sstream>
      using namespace std;
      #define USESTANDARD
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
      #include<strstream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   #include <sstream>
   using namespace std;
   #define USESTANDARD
#endif // sgi

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Interface/RunParameters.h"
#include "Interface/ProjectHandle.h"

#include "NumericFunctions.h"


using namespace DataAccess;
using namespace Interface;


RunParameters::RunParameters (ProjectHandle * projectHandle, Record * record) : DAObject (projectHandle, record)
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

   initialiseSeismicVelocityAlgorithm();
   
}

RunParameters::~RunParameters ()
{
}

void RunParameters::initialiseSeismicVelocityAlgorithm( ) {
   const std::string& seismicVelocityModelStr = database::getVelAlgorithm( m_record );
   if (seismicVelocityModelStr == "Gardner\'s Velocity-Density") {
      m_seismicVelocityModel = GARDNERS_VELOCITY_ALGORITHM;
   }
   else if (seismicVelocityModelStr == "Wyllie\'s Time-Average") {
      m_seismicVelocityModel = WYLLIES_VELOCITY_ALGORITHM;
   }
   else if (seismicVelocityModelStr == "Kennan\'s Velocity-Modulus") {
      m_seismicVelocityModel = KENNANS_VELOCITY_ALGORITHM;
   }
   else {
      std::cout << " Error in seismic velocity model " << seismicVelocityModelStr << ". Using Gardner seismic velocity as the default value." << endl;
      m_seismicVelocityModel = GARDNERS_VELOCITY_ALGORITHM;
   }
}

const std::string& RunParameters::getVreAlgorithm () const {
   return database::getVreAlgorithm ( m_record );
}

SeismicVelocityModel RunParameters::getSeismicVelocityAlgorithm() const {
	return m_seismicVelocityModel;
}

const std::string& RunParameters::getChemicalCompactionAlgorithm () const{
   return database::getChemicalCompactionAlgorithm ( m_record );
}

double RunParameters::getPrefReconstep () const {
   return database::getPrefReconstep ( m_record );
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

bool RunParameters::getConstrainVES () const {
   return database::getConstrain_VES ( m_record ) == 1;
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
   return database::getStepsPerSegment ( m_record );
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

#ifdef USESTANDARD
   ostringstream buf;
#else
   strstream buf;
#endif

   buf << "RunParameters:";
   buf << endl;

   str = buf.str ();
#ifndef USESTANDARD
   buf.rdbuf ()->freeze (0);
#endif

}
