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


#include <string>
using namespace std;

#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Interface/ProjectHandle.h"
#include "Interface/TrapPhase.h"

using namespace DataAccess;
using namespace Interface;

TrapPhase::TrapPhase (ProjectHandle * projectHandle, Record * record) : DAObject (projectHandle, record)
{
}

TrapPhase::~TrapPhase (void)
{
}

double TrapPhase::getPorosity (void) const
{
   return database::getPorosity (m_record);
}

double TrapPhase::getSealPermeability (void) const
{
   return database::getSealPermeability (m_record);
}

double TrapPhase::getGasWetness (void) const
{
   return database::getGasWetness (m_record);
}

double TrapPhase::getN2GasRatio (void) const
{
   return database::getN2GasRatio (m_record);
}

double TrapPhase::getAPIGravity (void) const
{
   return database::getAPI (m_record);
}

double TrapPhase::getCGR (void) const
{
   return database::getCGR (m_record);
}

double TrapPhase::getFormVolFactor (void) const
{
   return database::getFormVolFactor (m_record);
}

double TrapPhase::getGasMass (void) const
{
   return database::getGasMass (m_record);
}

double TrapPhase::getOilMass (void) const
{
   return database::getOilMass (m_record);
}

double TrapPhase::getGasDensity (void) const
{
   return database::getGasDensity (m_record);
}

double TrapPhase::getOilDensity (void) const
{
   return database::getOilDensity (m_record);
}

double TrapPhase::getGasViscosity (void) const
{
   return database::getGasViscosity (m_record);
}

double TrapPhase::getOilViscosity (void) const
{
   return database::getOilViscosity (m_record);
}

double TrapPhase::getGasVolume (void) const
{
   return database::getGasVolume (m_record);
}

double TrapPhase::getOilVolume (void) const
{
   return database::getOilVolume (m_record);
}


void TrapPhase::asString (string & str) const
{
#ifdef USESTANDARD
   ostringstream buf;
#else
   strstream buf;
#endif

   buf << "TrapPhase:";
   buf << endl;

   str = buf.str ();
#ifndef USESTANDARD
   buf.rdbuf ()->freeze (0);
#endif
}
