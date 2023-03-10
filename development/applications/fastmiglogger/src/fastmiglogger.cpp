//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// std library
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include <string>
#include <vector>
using namespace std;

// DataAccess API includes
#include "Interface.h"
#include "ProjectHandle.h"
#include "ObjectFactory.h"
#include "Snapshot.h"
#include "Trap.h"
#include "Surface.h"
#include "Formation.h"
#include "Reservoir.h"
#include "Grid.h"
using namespace DataAccess;
using namespace Interface;

// CBMGenerics library
#include "ComponentManager.h"
typedef CBMGenerics::ComponentManager::SpeciesNamesId ComponentId;
typedef CBMGenerics::ComponentManager::PhaseId PhaseId;

#include "EosPack.h"

#define Max(a,b)        (a > b ? a : b)
#define Min(a,b)        (a < b ? a : b)


static char * argv0 = 0;
static void showUsage(const char * message = 0);

const double stPressure = 0.101325;	// Stock tank pressure
const double stTemperature = 15.0;	// Stock tank temperature

const char *HLineA = "Age   |Trap|Approx |Approx |Trap  |Trap   |OWC |OWC  |GWC/|GWC/ |Approx |Approx   |Spill|Spill |Trap |Trap |Trap |Trap    |Gas   |Oil   |Oil   |Oil    |Gas   |Gas     \n";
const char *HLineB = "      |Id  |North  |East   |Height|Height |    |     |GOC |GOC  |WC Area|WC Area  |Depth|Depth |Temp |Temp |Press|Press   |I.T.  |I.T.  |Vol   |Vol    |Vol   |Vol     \n";
const char *HLineC = "Ma    |    |      m|      m|     m|     ft|   m|   ft|   m|   ft|    km2|     acre|    m|    ft|    C|    F|  MPa|    psia|   N/m|   N/m|10^6m3|10^6BBL|10^6m3|10^6ft3 \n";
const char *HLineD = "------|----|-------|-------|------|-------|----|-----|----|-----|-------|---------|-----|------|-----|-----|-----|--------|------|------|------|-------|------|------- \n";

const char *HLineA1 = "Age   |Trap|Trap      |Trap |Trap | Gas    |Oil\n";
const char *HLineB1 = "      |Id  |Perm.     |C1   |C2   | C.E.P. |C.E.P.\n";
const char *HLineC1 = "Ma    |    |          |     |     |      Pa|      Pa\n";
const char *HLineD1 = "------|----|----------|-----|-----|--------|--------\n";

const char *HLineAA = "Age   |Trap|Oil   |Spec. |Oil     |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil   |Oil |Oil |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil    |";
const char *HLineBA = "      |Id  |Des.  |Grav. |Vis.    |C1   |C2   |C3   |C4   |C5   |N2   |Cox  |H2S  |C6+S |C6+A |C15+S|C14+S|LSC  |C15+AT|C15+AS|C15+SS|C6+BT|C6+DBT|C6+BP|C6+SS|C6+AS| Res. |Asph.|GORM   |";
const char *HLineCA = "    Ma|    | kg/m3| g/cm3|      cP|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|   g/kg|";
const char *HLineDA = "------|----|------|------|--------|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-------|";

const char *HLineAB = "Gas   |Spec. |Gas     |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas   \n";
const char *HLineBB = "Des.  |Grav. |Vis .   |C1   |C2   |C3   |C4   |C5   |N2   |Cox  |H2S  |C6+S |C6+A |C15+S|C15+A|LSC  |C15+AT|C15+AS|C15+SS|C6+BT|C6+DBT|C6+BP|C6+SS|C6+AS|Res. |Asph. |GORM   \n";
const char *HLineCB = " kg/m3| g/cm3|      cP|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|   g/kg\n";
const char *HLineDB = "------|------|--------|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|--------\n";

const char *RC_LineA = "Trap|Approx  |Approx  |Gridsize|Trap |Trap   |";
const char *RC_LineB = "Id  |North   |East    |        |Temp |Press  |";
const char *OIL_RC_LineC = "    |      ft|      ft|      ft|    F|   Psia|";
const char *GAS_RC_LineC = "    |       m|       m|       m|    C|    MPa|";
const char *RC_LineD = "----|--------|--------|--------|-----|-------|";

const char *RC_LineAA = "RC_Oil    |RC_Gas     |RC_Oil  |RC_Gas  |RC_Oil  |RC_Gas  |";
const char *RC_LineBA = "Vol       |Vol        |Dens    |Dens    |Visc    |Visc    |";
const char *OIL_RC_LineCA = "   10^6BBL|    10^6ft3|   g/dm3|   g/dm3|      cP|      cP|";
const char *GAS_RC_LineCA = "    10^6m3|     10^6m3|   kg/m3|   kg/m3|      cP|      cP|";
const char *RC_LineDA = "----------|-----------|--------|--------|--------|--------|";

const char *RC_LineAB = "Trap    |OWC     |GWC/    |Approx   |POR   |PERM  |N/G   |OIlSat|GasSat\n";
const char *RC_LineBB = "Height  |        |GOC     |WC Area  |      |      |      |      |    \n";
const char *OIL_RC_LineCB = "      ft|      ft|      ft|     acre|  Vol%%|    mD|     %%|     %%|     %%\n";
const char *GAS_RC_LineCB = "       m|       m|       m|      km2|  Vol%%|    mD|     %%|     %%|     %%\n";
const char *RC_LineDB = "--------|--------|--------|---------|------|------|------|------|------\n";


const char *ST_LineA = "Trap|Approx  |Approx  |Gridsize|ST_Oil    |ST_Gas     |ST_Oil |API   |ST_Gas |ST_Oil  |ST_Gas  |ST__N2|ST_Cox|ST__N2|ST_Cox|";
const char *ST_LineB = " Id |North   |East    |        |Vol       |Vol        |Dens   |      |Dens   |Visc    |Visc    |OilPha|OilPha|GasPha|GasPha|";
const char *OIL_ST_LineC = "    |      ft|      ft|      ft|   10^6BBL|    10^6ft3|  g/dm3|      |  g/dm3|      cP|      cP|     %%|     %%|     %%|     %%|";
const char *GAS_ST_LineC = "    |       m|       m|       m|    10^6m3|     10^6m3|  kg/m3|      |  kg/m3|      cP|      cP|     %%|     %%|     %%|     %%|";
const char *ST_LineD = "----|--------|--------|--------|----------|-----------|-------|------|-------|--------|--------|------|------|------|------|";

const char *ST_LineAB = "CGR        |GOR       |Bg        |Bo\n";
const char *ST_LineBB = "GasPha     |OilPha    |          |     \n";
const char *OIL_ST_LineCB = "BBL/10^6ft3|   ft3/BBL|   ft3/ft3|   BBL/BBL\n";
const char *GAS_ST_LineCB = "  m3/10^3m3|     m3/m3|     m3/m3|     m3/m3\n";
const char *ST_LineDB = "-----------|----------|----------|----------\n";

const char *E_LineA = "Trap|Approx |Approx |HCPhase |Oil   |Oil    |Gas     |Gas     |";
const char *E_LineB = "Id  |North  |East   |RC      |Vol.  |Vol.   |Vol.    |Vol     |";
const char *E_LineC = "    |      m|      m|        |10^6m3|10^6BBL|  10^6m3| 10^6ft3|";
const char *E_LineD = "----|-------|-------|--------|------|-------|--------|--------|";
const char *E_LineAA = "Oil    |Spec.  |API   |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |Oil  |";
const char *E_LineBA = "Des.   |Grav.  |      |C1   |C2   |C3   |C4   |C5   |N2   |Cox  |H2S  |C6+S |C6+A |C15+S|C15+A|LSC  |C15+AT|C15+AS|C15+SS|C6+BT|C6+DBT|C6+BP|C6+SS|C6+AS|Res. |Asph.|";
const char *E_LineCA = "  kg/m3| g/cm^3|      |Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|";
const char *E_LineDA = "-------|-------|------|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|";
const char *E_LineAB = "Gas    |Spec.  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas  |Gas\n";
const char *E_LineBB = "Des.   |Grav.  |C1   |C2   |C3   |C4   |C5   |N2   |Cox  |H2S  |C6+S |C6+A |C15+S|C15+A|LSC  |C15+AT|C15+AS|C15+SS|C6+BT|C6+DBT|C6+BP|C6+SS|C6+AS|Res. |Asph.\n";
const char *E_LineCB = "  kg/m3| g/cm^3|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%|Mass%%\n";
const char *E_LineDB = "-------|-------|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----\n";


bool verbose = false;
bool debug = false;

bool processReservoir(ProjectHandle *projectHandle, const Reservoir * reservoir, SnapshotList * snapshotList);
string createLogFileName(ProjectHandle * projectHandle, const Reservoir * reservoir);

double meters2feet(double meters)
{
   return 3.28084 * meters;
}

double meters2kilometers(double meters)
{
   return 1e-3 * meters;
}

double squaremeters2squarekilometers(double squaremeters)
{
   return 1e-6 * squaremeters;
}

double squaremeters2acres(double squaremeters)
{
   return 0.000247 * squaremeters;
}

double celsius2fahrenheit(double celsius)
{
   return celsius * 1.8 + 32;
}

double celsius2kelvin(double celsius)
{
   return celsius + 273.15;
}

double megapascal2psi(double mp)
{
   return 145.03774389728312 * mp;
}

double megapascal2pascal(double mp)
{
   return 1e6 * mp;
}

double cubicmeters2barrels(double cm)
{
   return 6.289811 * cm;
}

double cubicmeters2standardcubicfeet(double cm)
{
   return 35.3146624 * cm;
}

void getFormattedString(double value, char *format, char *destString)
{
   if( value < 0.0 )
      strcpy(destString, "N/A");
   else
      snprintf(destString, 32, format, value);
}

double percentage(double component, double total)
{
   if( total < 10 )
      return 0.0;
   else
      return (component / total) * 100;
}


int main(int argc, char ** argv)
{
   string projectFileName;

   if( (argv0 = strrchr(argv[0], '/')) != 0 )
   {
      ++argv0;
   }
   else
   {
      argv0 = argv[0];
   }

   int arg;
   for( arg = 1; arg < argc; arg++ )
   {
      if( strncmp(argv[arg], "-debug", Max(2, strlen(argv[arg]))) == 0 )
      {
         debug = true;
      }
      else if( strncmp(argv[arg], "-verbose", Max(2, strlen(argv[arg]))) == 0 )
      {
         verbose = true;
      }
      else if( strncmp(argv[arg], "-", 1) != 0 )
      {
         projectFileName = argv[arg];
      }
      else
      {
         showUsage();
         return -1;
      }
   }

   if( projectFileName == "" )
   {
      showUsage("No project file specified");
      return -1;
   }

#if 0
   ofstream outputFile;

   outputFile.open(outputFileName.c_str());
   if( outputFile.fail() )
      return -1;
#endif

   ObjectFactory* factory = new ObjectFactory();
   ProjectHandle *projectHandle = OpenCauldronProject(projectFileName, factory);

   if( projectHandle == 0 )
   {
      cerr << "Could not open project file " << projectFileName << endl;
      return -1;
   }

   SnapshotList * snapshotList = projectHandle->getSnapshots();

   if( !snapshotList )
   {
      cerr << "Project " << projectFileName << " does not contain any snapshots" << endl;
      return -1;
   }

   ReservoirList * reservoirList = projectHandle->getReservoirs();
   ReservoirList::iterator iter;
   for( iter = reservoirList->begin(); iter != reservoirList->end(); ++iter )
   {
      if( !processReservoir(projectHandle, *iter, snapshotList) )
      {
         cerr << argv0 << ": Could not produce a log file for reservoir " << (*iter)->getName() << endl;
         return -1;
      }
   }

   delete factory;
   delete snapshotList;
   delete reservoirList;
   return 0;
}

bool processReservoir(ProjectHandle * projectHandle, const Reservoir * reservoir, SnapshotList * snapshotList)
{
   const double undefinedValue = -1;

   const int CBM_NumComponents = CBMGenerics::ComponentManager::NUMBER_OF_SPECIES; //CBMGenerics::ComponentManager::NumberOfSpeciesToFlash;
   const int CBM_NumPhases = CBMGenerics::ComponentManager::NUMBER_OF_PHASES;

   if( verbose || debug ) cout << "Reservoir: " << reservoir->getName() << endl;
   string filePath = createLogFileName(projectHandle, reservoir);

   if( verbose ) cout << "Producing log file " << filePath << endl;

   FILE * fp = fopen(filePath.c_str(), "w");
   if( fp == 0 )
   {
      perror(filePath.c_str());
      return false;
   }

   fprintf(fp, HLineA);
   fprintf(fp, HLineB);
   fprintf(fp, HLineC);
   fprintf(fp, HLineD);

   SnapshotList::reverse_iterator snapshotIter;
   for( snapshotIter = snapshotList->rbegin(); snapshotIter != snapshotList->rend(); ++snapshotIter )
   {
      const Snapshot * snapshot = *snapshotIter;

      if( verbose )
      {
         cout << "\tproducing snapshot " << snapshot->getTime() << endl;
      }

      TrapList * trapList = projectHandle->getTraps(reservoir, snapshot, 0);

      TrapList::iterator trapIter;
      for( trapIter = trapList->begin(); trapIter != trapList->end(); ++trapIter )
      {
         const Trap * trap = *trapIter;

         double trapX, trapY;
         trap->getPosition(trapX, trapY);

         fprintf(fp, "%6.1lf|%4d|%7.0lf|%7.0lf|%6.0lf|%7.0lf|%4.0lf|%5.0lf|%4.0lf|%5.0lf|"
            "%7.0lf|%9.0lf|%5.0lf|%6.0lf|%5.0lf|%5.0lf|%5.0lf|%8.0lf|%6.0lf|%6.0lf|%6.0lf|%7.0lf|%6.0lf|%7.0lf\n",
            snapshot->getTime(), trap->getId(), trapY, trapX,
            trap->getDepth(), meters2feet(trap->getDepth()),
            trap->getOWC(), meters2feet(trap->getOWC()),
            trap->getGOC(), meters2feet(trap->getGOC()),
            squaremeters2squarekilometers(trap->getWCSurface()),
            squaremeters2acres(trap->getWCSurface()),
            trap->getSpillDepth(), meters2feet(trap->getSpillDepth()),
            trap->getTemperature(), celsius2fahrenheit(trap->getTemperature()),
            trap->getPressure(), megapascal2psi(trap->getPressure()),
            undefinedValue /* interfacial tension of gas */,
            undefinedValue /* interfacial tension of oil */,
            trap->getVolume(PhaseId::OIL) / 1e6, cubicmeters2barrels(trap->getVolume(PhaseId::OIL) / 1e6),
            trap->getVolume(PhaseId::GAS) / 1e6, cubicmeters2standardcubicfeet(trap->getVolume(PhaseId::GAS) / 1e6));
      }
      delete trapList;
   }
   fprintf(fp, HLineD);

   fprintf(fp, "\n\n");

   fprintf(fp, HLineA1);
   fprintf(fp, HLineB1);
   fprintf(fp, HLineC1);
   fprintf(fp, HLineD1);

   for( snapshotIter = snapshotList->rbegin(); snapshotIter != snapshotList->rend(); ++snapshotIter )
   {
      const Snapshot * snapshot = *snapshotIter;

      TrapList * trapList = projectHandle->getTraps(reservoir, snapshot, 0);

      TrapList::iterator trapIter;
      for( trapIter = trapList->begin(); trapIter != trapList->end(); ++trapIter )
      {
         const Trap * trap = *trapIter;

         fprintf(fp, "%6.1lf|%4d|%10.2le|%5.1lf|%5.1lf|%8.0lf|%8.0lf\n",
            snapshot->getTime(), trap->getId(), undefinedValue /* seal permeability */,
            undefinedValue /* trap->getC1 () */, undefinedValue /* trap->getC2 () */,
            undefinedValue /* trap->getSealStrength (Gas) */, undefinedValue /* trap->getSealStrength (Oil) */);

      }
      delete trapList;
   }

   fprintf(fp, HLineD1);

   fprintf(fp, "\n\n");

   fprintf(fp, HLineAA);
   fprintf(fp, HLineAB);
   fprintf(fp, HLineBA);
   fprintf(fp, HLineBB);
   fprintf(fp, HLineCA);
   fprintf(fp, HLineCB);
   fprintf(fp, HLineDA);
   fprintf(fp, HLineDB);

   for( snapshotIter = snapshotList->rbegin(); snapshotIter != snapshotList->rend(); ++snapshotIter )
   {
      int componentId;
      int phaseId;

      const Snapshot * snapshot = *snapshotIter;

      TrapList * trapList = projectHandle->getTraps(reservoir, snapshot, 0);

      TrapList::iterator trapIter;
      for( trapIter = trapList->begin(); trapIter != trapList->end(); ++trapIter )
      {
         const Trap * trap = *trapIter;

         double inputMasses[CBM_NumComponents];
         double outputMasses[CBM_NumPhases][CBM_NumComponents];
         double outputDensities[CBM_NumPhases];
         double outputViscosities[CBM_NumPhases];

         for( componentId = 0; componentId < CBM_NumComponents; ++componentId )
         {
            inputMasses[componentId] = trap->getMass((ComponentId)componentId);
         }

         bool flashed = pvtFlash::EosPack::getInstance().computeWithLumping(celsius2kelvin(trap->getTemperature()),
            megapascal2pascal(trap->getPressure()),
            inputMasses, outputMasses, outputDensities, outputViscosities);

         if( flashed )
         {
            double phaseMasses[CBM_NumPhases];

            for( phaseId = 0; phaseId < CBM_NumPhases; ++phaseId )
            {
               phaseMasses[phaseId] = 0;
               for( componentId = 0; componentId < CBM_NumComponents; ++componentId )
               {
                  phaseMasses[phaseId] += outputMasses[phaseId][componentId];
               }
            }

            char gasViscString[32], oilViscString[32];

            getFormattedString(outputViscosities[PhaseId::GAS], "%1.2le", gasViscString);
            getFormattedString(outputViscosities[PhaseId::OIL], "%1.2le", oilViscString);

            char gasDensityString[32], gasGravityString[32], oilDensityString[32], oilGravityString[32];

            getFormattedString(outputDensities[PhaseId::GAS], "%6.1lf", gasDensityString);
            getFormattedString(outputDensities[PhaseId::OIL], "%6.1lf", oilDensityString);
            getFormattedString(0.001 * outputDensities[PhaseId::GAS], "%6.3lf", gasGravityString);
            getFormattedString(0.001 * outputDensities[PhaseId::OIL], "%6.3lf", oilGravityString);

            float num, deno, gasGorm, oilGorm;
            char gasGormString[32], oilGormString[32];

            num = outputMasses[PhaseId::GAS][ComponentId::C1] + outputMasses[PhaseId::GAS][ComponentId::C2] + outputMasses[PhaseId::GAS][ComponentId::C3] + outputMasses[PhaseId::GAS][ComponentId::C4] + outputMasses[PhaseId::GAS][ComponentId::C5];

            deno = outputMasses[PhaseId::GAS][ComponentId::C6_MINUS_14SAT  ] + outputMasses[PhaseId::GAS][ComponentId::C6_MINUS_14ARO  ] +
                   outputMasses[PhaseId::GAS][ComponentId::C15_PLUS_SAT    ] + outputMasses[PhaseId::GAS][ComponentId::C15_PLUS_ARO    ] +
                   outputMasses[PhaseId::GAS][ComponentId::C15_PLUS_SAT_S  ] + outputMasses[PhaseId::GAS][ComponentId::C15_PLUS_ARO_S  ] +
                   outputMasses[PhaseId::GAS][ComponentId::C6_MINUS_14ARO_S] + outputMasses[PhaseId::GAS][ComponentId::C6_MINUS_14SAT_S] +
                   outputMasses[PhaseId::GAS][ComponentId::C15_PLUS_AT     ] + outputMasses[PhaseId::GAS][ComponentId::LSC             ] +
                   outputMasses[PhaseId::GAS][ComponentId::C6_MINUS_14DBT  ] + outputMasses[PhaseId::GAS][ComponentId::C6_MINUS_14BT   ] +
                   outputMasses[PhaseId::GAS][ComponentId::C6_MINUS_14BP   ] +
                   outputMasses[PhaseId::GAS][ComponentId::RESIN           ] +
                   outputMasses[PhaseId::GAS][ComponentId::ASPHALTENE];

            gasGorm = (deno) ? num / deno : -1;
            getFormattedString(1000 * gasGorm, "%8.1lf", gasGormString);

            num = outputMasses[PhaseId::OIL][ComponentId::C1] + outputMasses[PhaseId::OIL][ComponentId::C2] + outputMasses[PhaseId::OIL][ComponentId::C3] + outputMasses[PhaseId::OIL][ComponentId::C4] + outputMasses[PhaseId::OIL][ComponentId::C5];

            deno = outputMasses[PhaseId::OIL][ComponentId::C6_MINUS_14SAT    ] + outputMasses[PhaseId::OIL][ComponentId::C6_MINUS_14ARO  ] +
                   outputMasses[PhaseId::OIL][ComponentId::C15_PLUS_SAT      ] + outputMasses[PhaseId::OIL][ComponentId::C15_PLUS_ARO    ] +
                   outputMasses[PhaseId::OIL][ComponentId::C15_PLUS_SAT_S    ] + outputMasses[PhaseId::OIL][ComponentId::C15_PLUS_ARO_S  ] +
                   outputMasses[PhaseId::OIL][ComponentId::C6_MINUS_14ARO_S  ] + outputMasses[PhaseId::OIL][ComponentId::C6_MINUS_14SAT_S] +
                   outputMasses[PhaseId::OIL][ComponentId::C15_PLUS_AT       ] + outputMasses[PhaseId::OIL][ComponentId::LSC             ] +
                   outputMasses[PhaseId::OIL][ComponentId::C6_MINUS_14DBT    ] + outputMasses[PhaseId::OIL][ComponentId::C6_MINUS_14BT   ] +
                   outputMasses[PhaseId::OIL][ComponentId::C6_MINUS_14BP     ] +
                   outputMasses[PhaseId::OIL][ComponentId::RESIN             ] +
                   outputMasses[PhaseId::OIL][ComponentId::ASPHALTENE];

            oilGorm = (deno) ? num / deno : -1;
            getFormattedString(1000 * oilGorm, "%7.1f", oilGormString);

            fprintf(fp, "%6.1lf|%4d|%6s|%6s|%8s|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|"
               "%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%7s|%6s|%6s|%8s|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|"
               "%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%s\n",
               snapshot->getTime(), trap->getId(),
               oilDensityString, oilGravityString, oilViscString,
               percentage(outputMasses[PhaseId::OIL][ComponentId::C1            ], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::C2            ], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::C3            ], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::C4            ], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::C5            ], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::N2            ], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::COX           ], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::H2S           ], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::C6_MINUS_14SAT], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::C6_MINUS_14ARO], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::C15_PLUS_SAT  ], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::C15_PLUS_ARO  ], phaseMasses[PhaseId::OIL]),

               percentage(outputMasses[PhaseId::OIL][ComponentId::LSC             ], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::C15_PLUS_AT     ], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::C15_PLUS_ARO_S  ], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::C15_PLUS_SAT_S  ], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::C6_MINUS_14BT   ], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::C6_MINUS_14DBT  ], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::C6_MINUS_14BP   ], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::C6_MINUS_14SAT_S], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::C6_MINUS_14ARO_S], phaseMasses[PhaseId::OIL]),

               percentage(outputMasses[PhaseId::OIL][ComponentId::RESIN     ], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::ASPHALTENE], phaseMasses[PhaseId::OIL]),
               oilGormString,
               gasDensityString, gasGravityString, gasViscString,
               percentage(outputMasses[PhaseId::GAS][ComponentId::C1            ], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::C2            ], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::C3            ], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::C4            ], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::C5            ], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::N2            ], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::COX           ], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::H2S           ], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::C6_MINUS_14SAT], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::C6_MINUS_14ARO], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::C15_PLUS_SAT  ], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::C15_PLUS_ARO  ], phaseMasses[PhaseId::GAS]),

               percentage(outputMasses[PhaseId::GAS][ComponentId::LSC             ], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::C15_PLUS_AT     ], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::C15_PLUS_ARO_S  ], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::C15_PLUS_SAT_S  ], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::C6_MINUS_14BT   ], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::C6_MINUS_14DBT  ], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::C6_MINUS_14BP   ], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::C6_MINUS_14SAT_S], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::C6_MINUS_14ARO_S], phaseMasses[PhaseId::GAS]),


               percentage(outputMasses[PhaseId::GAS][ComponentId::RESIN     ], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::ASPHALTENE], phaseMasses[PhaseId::GAS]),
               gasGormString);
         }
      }
      delete trapList;
   }

   fprintf(fp, HLineDA);
   fprintf(fp, HLineDB);

   fprintf(fp,
      "\nPVT parameters used for phase properties and volumes "
      "are at reservoir conditions.\n");

   fprintf(fp, "\n\n");

   fprintf(fp, "[FASTRACK_CAULDRON3D_BEGIN_RC]\n");

   fprintf(fp, RC_LineA);
   fprintf(fp, RC_LineAA);
   fprintf(fp, RC_LineAB);
   fprintf(fp, RC_LineB);
   fprintf(fp, RC_LineBA);
   fprintf(fp, RC_LineBB);
   fprintf(fp, GAS_RC_LineC);
   fprintf(fp, GAS_RC_LineCA);
   fprintf(fp, GAS_RC_LineCB);

   fprintf(fp, RC_LineD);
   fprintf(fp, RC_LineDA);
   fprintf(fp, RC_LineDB);


   {
      int componentId;
      int phaseId;

      const Snapshot * snapshot = (*snapshotList)[0];

      TrapList * trapList = projectHandle->getTraps(reservoir, snapshot, 0);

      TrapList::iterator trapIter;
      for( trapIter = trapList->begin(); trapIter != trapList->end(); ++trapIter )
      {
         const Trap * trap = *trapIter;

         double inputMasses[CBM_NumComponents];
         double outputMasses[CBM_NumPhases][CBM_NumComponents];
         double outputDensities[CBM_NumPhases];
         double outputViscosities[CBM_NumPhases];

         for( componentId = 0; componentId < CBM_NumComponents; ++componentId )
         {
            inputMasses[componentId] = trap->getMass((ComponentId)componentId);
         }

         bool flashed = pvtFlash::EosPack::getInstance().computeWithLumping(celsius2kelvin(trap->getTemperature()),
            megapascal2pascal(trap->getPressure()),
            inputMasses, outputMasses, outputDensities, outputViscosities);

         if( flashed )
         {
            double phaseMasses[CBM_NumPhases];

            for( phaseId = 0; phaseId < CBM_NumPhases; ++phaseId )
            {
               phaseMasses[phaseId] = 0;
               for( componentId = 0; componentId < CBM_NumComponents; ++componentId )
               {
                  phaseMasses[phaseId] += outputMasses[phaseId][componentId];
               }
            }

            double trapX, trapY;
            trap->getPosition(trapX, trapY);

            fprintf(fp, "%4d|%8.0lf|%8.0lf|%8.0lf|%5.0lf|%7.0lf|",
               trap->getId(), trapY, trapX, projectHandle->getHighResolutionOutputGrid()->deltaI(), trap->getTemperature(), trap->getPressure());

            double oilVolume = 0;
            if( phaseMasses[PhaseId::OIL] > 0 && outputDensities[PhaseId::OIL] > 0 )
               oilVolume = phaseMasses[PhaseId::OIL] / outputDensities[PhaseId::OIL];

            double gasVolume = 0;
            if( phaseMasses[PhaseId::GAS] > 0 && outputDensities[PhaseId::GAS] > 0 )
               gasVolume = phaseMasses[PhaseId::GAS] / outputDensities[PhaseId::GAS];

            char gasDensityString[32], oilDensityString[32];

            getFormattedString(outputDensities[PhaseId::GAS], "%1.2le", gasDensityString);
            getFormattedString(outputDensities[PhaseId::OIL], "%1.2le", oilDensityString);

            char gasViscString[32], oilViscString[32];

            getFormattedString(outputViscosities[PhaseId::GAS], "%1.2le", gasViscString);
            getFormattedString(outputViscosities[PhaseId::OIL], "%1.2le", oilViscString);

            fprintf(fp, "%10.2lf|%11.2lf|%7s|%7s|%8s|%8s|",
               oilVolume / 1e6, gasVolume / 1e6, oilDensityString, gasDensityString, oilViscString, gasViscString);

            fprintf(fp, "%8.1lf|%8.1lf|%8.1lf|%9.1lf|",
               trap->getDepth(), trap->getOWC(), trap->getGOC(), squaremeters2squarekilometers(trap->getWCSurface()));

            double porosity = -1;
            double perm = -1;
            double net_gross = 100;
            double oilsat = 100;
            double gassat = 100;

            char porosityString[32];
            getFormattedString(porosity, "%6.1lf", porosityString);

            char permString[32];
            getFormattedString(perm, "%6.1lf", permString);

            fprintf(fp, "%6s|%6s|%6.1lf|%6.1lf|%6.1lf\n",
               porosityString, permString, net_gross, oilsat, gassat);
         }
      }
      delete trapList;
   }

   fprintf(fp, RC_LineD);
   fprintf(fp, RC_LineDA);
   fprintf(fp, RC_LineDB);

   fprintf(fp, "[FASTRACK_CAULDRON3D_END_RC]\n");
   fprintf(fp, "NB: -1.0  values stands for \"undef\"\n");


   fprintf(fp, "\n\n");
   fprintf(fp, "[FASTRACK_CAULDRON3D_BEGIN_RC OIL FIELD UNITS]\n");

   fprintf(fp, RC_LineA);
   fprintf(fp, RC_LineAA);
   fprintf(fp, RC_LineAB);
   fprintf(fp, RC_LineB);
   fprintf(fp, RC_LineBA);
   fprintf(fp, RC_LineBB);
   fprintf(fp, OIL_RC_LineC);
   fprintf(fp, OIL_RC_LineCA);
   fprintf(fp, OIL_RC_LineCB);

   fprintf(fp, RC_LineD);
   fprintf(fp, RC_LineDA);
   fprintf(fp, RC_LineDB);

   {
      int componentId;
      int phaseId;

      const Snapshot * snapshot = (*snapshotList)[0];

      TrapList * trapList = projectHandle->getTraps(reservoir, snapshot, 0);

      TrapList::iterator trapIter;
      for( trapIter = trapList->begin(); trapIter != trapList->end(); ++trapIter )
      {
         const Trap * trap = *trapIter;

         double inputMasses[CBM_NumComponents];
         double outputMasses[CBM_NumPhases][CBM_NumComponents];
         double outputDensities[CBM_NumPhases];
         double outputViscosities[CBM_NumPhases];

         for( componentId = 0; componentId < CBM_NumComponents; ++componentId )
         {
            inputMasses[componentId] = trap->getMass((ComponentId)componentId);
         }

         bool flashed = pvtFlash::EosPack::getInstance().computeWithLumping(celsius2kelvin(trap->getTemperature()),
            megapascal2pascal(trap->getPressure()),
            inputMasses, outputMasses, outputDensities, outputViscosities);

         if( flashed )
         {
            double phaseMasses[CBM_NumPhases];

            for( phaseId = 0; phaseId < CBM_NumPhases; ++phaseId )
            {
               phaseMasses[phaseId] = 0;
               for( componentId = 0; componentId < CBM_NumComponents; ++componentId )
               {
                  phaseMasses[phaseId] += outputMasses[phaseId][componentId];
               }
            }

            double trapX, trapY;
            trap->getPosition(trapX, trapY);

            fprintf(fp, "%4d|%8.0lf|%8.0lf|%8.0lf|%5.0lf|%7.0lf|",
               trap->getId(), meters2feet(trapY), meters2feet(trapX), meters2feet(projectHandle->getHighResolutionOutputGrid()->deltaI()),
               celsius2fahrenheit(trap->getTemperature()), megapascal2psi(trap->getPressure()));

            double oilVolume = 0;
            if( phaseMasses[PhaseId::OIL] > 0 && outputDensities[PhaseId::OIL] > 0 )
               oilVolume = phaseMasses[PhaseId::OIL] / outputDensities[PhaseId::OIL];

            double gasVolume = 0;
            if( phaseMasses[PhaseId::GAS] > 0 && outputDensities[PhaseId::GAS] > 0 )
               gasVolume = phaseMasses[PhaseId::GAS] / outputDensities[PhaseId::GAS];

            char gasDensityString[32], oilDensityString[32];

            getFormattedString(outputDensities[PhaseId::GAS], "%1.2le", gasDensityString);
            getFormattedString(outputDensities[PhaseId::OIL], "%1.2le", oilDensityString);

            char gasViscString[32], oilViscString[32];

            getFormattedString(outputViscosities[PhaseId::GAS], "%1.2le", gasViscString);
            getFormattedString(outputViscosities[PhaseId::OIL], "%1.2le", oilViscString);

            fprintf(fp, "%10.2lf|%11.2lf|%7s|%7s|%8s|%8s|",
               cubicmeters2barrels(oilVolume / 1e6), cubicmeters2standardcubicfeet(gasVolume / 1e6), oilDensityString, gasDensityString, oilViscString, gasViscString);

            fprintf(fp, "%8.1lf|%8.1lf|%8.1lf|%9.1lf|",
               meters2feet(trap->getDepth()), meters2feet(trap->getOWC()), meters2feet(trap->getGOC()), squaremeters2acres(trap->getWCSurface()));

            double porosity = -1;
            double perm = -1;
            double net_gross = 100;
            double oilsat = 100;
            double gassat = 100;

            char porosityString[32];
            getFormattedString(porosity, "%6.1lf", porosityString);

            char permString[32];
            getFormattedString(perm, "%6.1lf", permString);

            fprintf(fp, "%6s|%6s|%6.1lf|%6.1lf|%6.1lf\n",
               porosityString, permString, net_gross, oilsat, gassat);
         }
      }
      delete trapList;
   }

   fprintf(fp, RC_LineD);
   fprintf(fp, RC_LineDA);
   fprintf(fp, RC_LineDB);

   fprintf(fp, "\n\n");

   fprintf(fp, "[FASTRACK_CAULDRON3D_END_RC OIL FIELD UNIT]\n");
   fprintf(fp, "NB: -1.0  values stands for \"undef\"\n");


   fprintf(fp, "\n\n\n\n\n");
   fprintf(fp, "[FASTRACK_CAULDRON3D_BEGIN_ST]\n");

   fprintf(fp, ST_LineA);
   fprintf(fp, ST_LineAB);
   fprintf(fp, ST_LineB);
   fprintf(fp, ST_LineBB);
   fprintf(fp, GAS_ST_LineC);
   fprintf(fp, GAS_ST_LineCB);

   fprintf(fp, ST_LineD);
   fprintf(fp, ST_LineDB);

   {
      int componentId;
      int phaseId;

      char * format = "%4d|%8.0lf|%8.0lf|%8.0lf|%10.1lf|%11.1lf|%7s|%6s|%7s|%8s|%8s|%6.1lf|%6.1lf|%6.1lf|%6.1lf|";

      const Snapshot * snapshot = (*snapshotList)[0];

      TrapList * trapList = projectHandle->getTraps(reservoir, snapshot, 0);

      TrapList::iterator trapIter;
      for( trapIter = trapList->begin(); trapIter != trapList->end(); ++trapIter )
      {
         bool flashed;
         const Trap * trap = *trapIter;

         double inputMasses[CBM_NumComponents];

         double outputMasses[CBM_NumPhases][CBM_NumComponents];
         double outputDensities[CBM_NumPhases];
         double outputViscosities[CBM_NumPhases];

         double stOutputMasses[CBM_NumPhases][CBM_NumComponents];
         double stOutputDensities[CBM_NumPhases];
         double stOutputViscosities[CBM_NumPhases];

         for( componentId = 0; componentId < CBM_NumComponents; ++componentId )
         {
            inputMasses[componentId] = trap->getMass((ComponentId)componentId);
         }

         // flash everything at reservoir conditions
         flashed = pvtFlash::EosPack::getInstance().computeWithLumping(celsius2kelvin(trap->getTemperature()),
            megapascal2pascal(trap->getPressure()),
            inputMasses, outputMasses, outputDensities, outputViscosities);

         if( !flashed ) continue;

         // flash everything at stock tank conditions
         flashed = pvtFlash::EosPack::getInstance().computeWithLumping(celsius2kelvin(stTemperature),
            megapascal2pascal(stPressure),
            inputMasses, stOutputMasses, stOutputDensities, stOutputViscosities);

         if( !flashed ) continue;

         double gasInputMasses[CBM_NumComponents];
         double gasOutputMasses[CBM_NumPhases][CBM_NumComponents];
         double gasOutputDensities[CBM_NumPhases];
         double gasOutputViscosities[CBM_NumPhases];

         for( componentId = 0; componentId < CBM_NumComponents; ++componentId )
         {
            gasInputMasses[componentId] = outputMasses[PhaseId::GAS][componentId];
         }

         // flash reservoir gas at stock tank conditions
         flashed = pvtFlash::EosPack::getInstance().computeWithLumping(celsius2kelvin(stTemperature),
            megapascal2pascal(stPressure),
            gasInputMasses, gasOutputMasses, gasOutputDensities, gasOutputViscosities);

         if( !flashed ) continue;

         double oilInputMasses[CBM_NumComponents];
         double oilOutputMasses[CBM_NumPhases][CBM_NumComponents];
         double oilOutputDensities[CBM_NumPhases];
         double oilOutputViscosities[CBM_NumPhases];

         for( componentId = 0; componentId < CBM_NumComponents; ++componentId )
         {
            oilInputMasses[componentId] = outputMasses[PhaseId::OIL][componentId];
         }

         // flash reservoir oil at stock tank conditions
         flashed = pvtFlash::EosPack::getInstance().computeWithLumping(celsius2kelvin(stTemperature),
            megapascal2pascal(stPressure),
            oilInputMasses, oilOutputMasses, oilOutputDensities, oilOutputViscosities);

         if( !flashed ) continue;

         double phaseMasses[CBM_NumPhases];
         double gasPhaseMasses[CBM_NumPhases];
         double oilPhaseMasses[CBM_NumPhases];
         double stPhaseMasses[CBM_NumPhases];

         for( phaseId = 0; phaseId < CBM_NumPhases; ++phaseId )
         {
            phaseMasses[phaseId] = 0;
            gasPhaseMasses[phaseId] = 0;
            oilPhaseMasses[phaseId] = 0;
            stPhaseMasses[phaseId] = 0;

            for( componentId = 0; componentId < CBM_NumComponents; ++componentId )
            {
               phaseMasses[phaseId] += outputMasses[phaseId][componentId];
               gasPhaseMasses[phaseId] += gasOutputMasses[phaseId][componentId];
               oilPhaseMasses[phaseId] += oilOutputMasses[phaseId][componentId];
               stPhaseMasses[phaseId] += stOutputMasses[phaseId][componentId];
            }
         }

         double trapX, trapY;
         trap->getPosition(trapX, trapY);

         double oilVolume = 0;
         double API = -1;
         if( stPhaseMasses[PhaseId::OIL] > 0 && stOutputDensities[PhaseId::OIL] > 0 )
         {
            oilVolume = stPhaseMasses[PhaseId::OIL] / stOutputDensities[PhaseId::OIL];
            API = 141.5 / (0.001*stOutputDensities[PhaseId::OIL]) - 131.5;
         }

         double gasVolume = 0;
         if( phaseMasses[PhaseId::GAS] > 0 && outputDensities[PhaseId::GAS] > 0 )
         {
            gasVolume = stPhaseMasses[PhaseId::GAS] / stOutputDensities[PhaseId::GAS];
         }

         char oilDensityString[32], gasDensityString[32];
         getFormattedString(stOutputDensities[PhaseId::OIL], "%7.1lf", oilDensityString);
         getFormattedString(stOutputDensities[PhaseId::GAS], "%7.1lf", gasDensityString);

         char APIString[32];
         getFormattedString(API, "%6.1lf", APIString);

         char oilViscString[32], gasViscString[32];
         getFormattedString(stOutputViscosities[PhaseId::OIL], "%1.2le", oilViscString);
         getFormattedString(stOutputViscosities[PhaseId::GAS], "%1.2le", gasViscString);

         fprintf(fp, format,
            trap->getId(), trapY, trapX, projectHandle->getHighResolutionOutputGrid()->deltaI(),
            oilVolume / 1e6, gasVolume / 1e6,
            oilDensityString, APIString, gasDensityString, oilViscString, gasViscString,
            percentage(stOutputMasses[PhaseId::OIL][ComponentId::N2],  stPhaseMasses[PhaseId::OIL]),
            percentage(stOutputMasses[PhaseId::OIL][ComponentId::COX], stPhaseMasses[PhaseId::OIL]),
            percentage(stOutputMasses[PhaseId::GAS][ComponentId::N2],  stPhaseMasses[PhaseId::GAS]),
            percentage(stOutputMasses[PhaseId::GAS][ComponentId::COX], stPhaseMasses[PhaseId::GAS]));


         double condensate = 0;
         if( gasPhaseMasses[PhaseId::OIL] > 0 && gasOutputDensities[PhaseId::OIL] > 0 )
            condensate = gasPhaseMasses[PhaseId::OIL] / gasOutputDensities[PhaseId::OIL];

         double gas = 0;
         if( gasPhaseMasses[PhaseId::GAS] > 0 && gasOutputDensities[PhaseId::GAS] > 0 )
            gas = gasPhaseMasses[PhaseId::GAS] / gasOutputDensities[PhaseId::GAS];

         double cgr = 0;
         double bg = 0;

         if( condensate > 0 && gas > 0 )
         {
            cgr = condensate / gas;
            bg = phaseMasses[PhaseId::GAS] / outputDensities[PhaseId::GAS] / gas;
         }
         else
         {
            cgr = -1;
            bg = -1;
         }

         char cgrString[32];
         getFormattedString((cgr * 1000.0), "%11.2le", cgrString);


         gas = 0;
         if( oilPhaseMasses[PhaseId::GAS] > 0 && oilOutputDensities[PhaseId::GAS] > 0 )
            gas = oilPhaseMasses[PhaseId::GAS] / oilOutputDensities[PhaseId::GAS];

         double oil = 0;
         double gor = 0;
         double bo = 0;
         if( oilPhaseMasses[PhaseId::OIL] > 0 && oilOutputDensities[PhaseId::OIL] > 0 )
         {
            oil = oilPhaseMasses[PhaseId::OIL] / oilOutputDensities[PhaseId::OIL];
            bo = phaseMasses[PhaseId::OIL] / outputDensities[PhaseId::OIL] / oil;
            gor = gas / oil;
         }

         char gorString[32], bgString[32], boString[32];
         getFormattedString(gor, "%10.2le", gorString);
         getFormattedString(bg, "%10.2le", bgString);
         getFormattedString(bo, "%10.2le", boString);

         fprintf(fp, "%11s|%10s|%10s|%10s\n",
            cgrString, gorString, bgString, boString);
      }
      delete trapList;
   }

   fprintf(fp, ST_LineD);
   fprintf(fp, ST_LineDB);

   fprintf(fp, "[FASTRACK_CAULDRON3D_END_ST]\n");
   fprintf(fp, "NB: -1.0 values stands for \"undef\"\n");

   fprintf(fp, "\n\n");
   fprintf(fp, "[FASTRACK_CAULDRON3D_BEGIN_ST OIL FIELD UNITS]\n");

   fprintf(fp, ST_LineA);
   fprintf(fp, ST_LineAB);
   fprintf(fp, ST_LineB);
   fprintf(fp, ST_LineBB);
   fprintf(fp, OIL_ST_LineC);
   fprintf(fp, OIL_ST_LineCB);

   fprintf(fp, ST_LineD);
   fprintf(fp, ST_LineDB);

   {
      int componentId;
      int phaseId;

      char * format = "%4d|%8.0lf|%8.0lf|%8.0lf|%10.1lf|%11.1lf|%7s|%6s|%7s|%8s|%8s|%6.1lf|%6.1lf|%6.1lf|%6.1lf|";

      const Snapshot * snapshot = (*snapshotList)[0];

      TrapList * trapList = projectHandle->getTraps(reservoir, snapshot, 0);

      TrapList::iterator trapIter;
      for( trapIter = trapList->begin(); trapIter != trapList->end(); ++trapIter )
      {
         bool flashed;
         const Trap * trap = *trapIter;

         double inputMasses[CBM_NumComponents];

         double outputMasses[CBM_NumPhases][CBM_NumComponents];
         double outputDensities[CBM_NumPhases];
         double outputViscosities[CBM_NumPhases];

         double stOutputMasses[CBM_NumPhases][CBM_NumComponents];
         double stOutputDensities[CBM_NumPhases];
         double stOutputViscosities[CBM_NumPhases];

         for( componentId = 0; componentId < CBM_NumComponents; ++componentId )
         {
            inputMasses[componentId] = trap->getMass((ComponentId)componentId);
         }

         // flash everything at reservoir conditions
         flashed = pvtFlash::EosPack::getInstance().computeWithLumping(celsius2kelvin(trap->getTemperature()),
            megapascal2pascal(trap->getPressure()),
            inputMasses, outputMasses, outputDensities, outputViscosities);

         if( !flashed ) continue;

         // flash everything at stock tank conditions
         flashed = pvtFlash::EosPack::getInstance().computeWithLumping(celsius2kelvin(stTemperature),
            megapascal2pascal(stPressure),
            inputMasses, stOutputMasses, stOutputDensities, stOutputViscosities);

         if( !flashed ) continue;

         double gasInputMasses[CBM_NumComponents];
         double gasOutputMasses[CBM_NumPhases][CBM_NumComponents];
         double gasOutputDensities[CBM_NumPhases];
         double gasOutputViscosities[CBM_NumPhases];

         for( componentId = 0; componentId < CBM_NumComponents; ++componentId )
         {
            gasInputMasses[componentId] = outputMasses[PhaseId::GAS][componentId];
         }

         // flash reservoir gas at stock tank conditions
         flashed = pvtFlash::EosPack::getInstance().computeWithLumping(celsius2kelvin(stTemperature),
            megapascal2pascal(stPressure),
            gasInputMasses, gasOutputMasses, gasOutputDensities, gasOutputViscosities);

         if( !flashed ) continue;

         double oilInputMasses[CBM_NumComponents];
         double oilOutputMasses[CBM_NumPhases][CBM_NumComponents];
         double oilOutputDensities[CBM_NumPhases];
         double oilOutputViscosities[CBM_NumPhases];

         for( componentId = 0; componentId < CBM_NumComponents; ++componentId )
         {
            oilInputMasses[componentId] = outputMasses[PhaseId::OIL][componentId];
         }

         // flash reservoir oil at stock tank conditions
         flashed = pvtFlash::EosPack::getInstance().computeWithLumping(celsius2kelvin(stTemperature),
            megapascal2pascal(stPressure),
            oilInputMasses, oilOutputMasses, oilOutputDensities, oilOutputViscosities);

         if( !flashed ) continue;

         double phaseMasses[CBM_NumPhases];
         double gasPhaseMasses[CBM_NumPhases];
         double oilPhaseMasses[CBM_NumPhases];
         double stPhaseMasses[CBM_NumPhases];

         for( phaseId = 0; phaseId < CBM_NumPhases; ++phaseId )
         {
            phaseMasses[phaseId] = 0;
            gasPhaseMasses[phaseId] = 0;
            oilPhaseMasses[phaseId] = 0;
            stPhaseMasses[phaseId] = 0;

            for( componentId = 0; componentId < CBM_NumComponents; ++componentId )
            {
               phaseMasses[phaseId] += outputMasses[phaseId][componentId];
               gasPhaseMasses[phaseId] += gasOutputMasses[phaseId][componentId];
               oilPhaseMasses[phaseId] += oilOutputMasses[phaseId][componentId];
               stPhaseMasses[phaseId] += stOutputMasses[phaseId][componentId];
            }
         }

         double trapX, trapY;
         trap->getPosition(trapX, trapY);

         double oilVolume = 0;
         double API = -1;
         if( stPhaseMasses[PhaseId::OIL] > 0 && stOutputDensities[PhaseId::OIL] > 0 )
         {
            oilVolume = stPhaseMasses[PhaseId::OIL] / stOutputDensities[PhaseId::OIL];
            API = 141.5 / (0.001*stOutputDensities[PhaseId::OIL]) - 131.5;
         }

         double gasVolume = 0;
         if( phaseMasses[PhaseId::GAS] > 0 && outputDensities[PhaseId::GAS] > 0 )
         {
            gasVolume = stPhaseMasses[PhaseId::GAS] / stOutputDensities[PhaseId::GAS];
         }

         char oilDensityString[32], gasDensityString[32];
         getFormattedString(stOutputDensities[PhaseId::OIL], "%7.3lf", oilDensityString);
         getFormattedString(stOutputDensities[PhaseId::GAS], "%7.3lf", gasDensityString);

         char APIString[32];
         getFormattedString(API, "%6.1lf", APIString);

         char oilViscString[32], gasViscString[32];
         getFormattedString(stOutputViscosities[PhaseId::OIL], "%1.2le", oilViscString);
         getFormattedString(stOutputViscosities[PhaseId::GAS], "%1.2le", gasViscString);

         fprintf(fp, format,
            trap->getId(), meters2feet(trapY), meters2feet(trapX), meters2feet(projectHandle->getHighResolutionOutputGrid()->deltaI()),
            cubicmeters2barrels(oilVolume / 1e6), cubicmeters2standardcubicfeet(gasVolume / 1e6),
            oilDensityString, APIString, gasDensityString, oilViscString, gasViscString,
            percentage(stOutputMasses[PhaseId::OIL][ComponentId::N2], stPhaseMasses[PhaseId::OIL]),
            percentage(stOutputMasses[PhaseId::OIL][ComponentId::COX], stPhaseMasses[PhaseId::OIL]),
            percentage(stOutputMasses[PhaseId::GAS][ComponentId::N2], stPhaseMasses[PhaseId::GAS]),
            percentage(stOutputMasses[PhaseId::GAS][ComponentId::COX], stPhaseMasses[PhaseId::GAS]));


         double condensate = 0;
         if( gasPhaseMasses[PhaseId::OIL] > 0 && gasOutputDensities[PhaseId::OIL] > 0 )
            condensate = gasPhaseMasses[PhaseId::OIL] / gasOutputDensities[PhaseId::OIL];

         double gas = 0;
         if( gasPhaseMasses[PhaseId::GAS] > 0 && gasOutputDensities[PhaseId::GAS] > 0 )
            gas = gasPhaseMasses[PhaseId::GAS] / gasOutputDensities[PhaseId::GAS];

         double cgr = 0;
         double bg = 0;

         if( condensate > 0 && gas > 0 )
         {
            cgr = condensate / gas;
            bg = phaseMasses[PhaseId::GAS] / outputDensities[PhaseId::GAS] / gas;
         }
         else
         {
            cgr = -1;
            bg = -1;
         }

         cgr *= 1000 * cubicmeters2barrels(1) / cubicmeters2standardcubicfeet(1);
         char cgrString[32];
         getFormattedString((cgr * 1000.0), "%11.2le", cgrString);

         gas = 0;
         if( oilPhaseMasses[PhaseId::GAS] > 0 && oilOutputDensities[PhaseId::GAS] > 0 )
            gas = oilPhaseMasses[PhaseId::GAS] / oilOutputDensities[PhaseId::GAS];

         double oil = 0;
         double gor = 0;
         double bo = 0;
         if( oilPhaseMasses[PhaseId::OIL] > 0 && oilOutputDensities[PhaseId::OIL] > 0 )
         {
            oil = oilPhaseMasses[PhaseId::OIL] / oilOutputDensities[PhaseId::OIL];
            bo = phaseMasses[PhaseId::OIL] / outputDensities[PhaseId::OIL] / oil;
            gor = gas / oil;
         }

         gor *= cubicmeters2standardcubicfeet(1) / cubicmeters2barrels(1);
         char gorString[32], bgString[32], boString[32];
         getFormattedString(gor, "%10.2le", gorString);
         getFormattedString(bg, "%10.2le", bgString);
         getFormattedString(bo, "%10.2le", boString);

         fprintf(fp, "%11s|%10s|%10s|%10s\n",
            cgrString, gorString, bgString, boString);
      }
      delete trapList;
   }


   fprintf(fp, ST_LineD);
   fprintf(fp, ST_LineDB);

   fprintf(fp, "[FASTRACK_CAULDRON3D_END_ST OIL FIELD UNITS]\n");
   fprintf(fp, "NB: -1.0 values stands for \"undef\"\n");

   fprintf(fp, "\n\n\n\n\n");
   fprintf(fp, E_LineA);
   fprintf(fp, E_LineAA);
   fprintf(fp, E_LineAB);
   fprintf(fp, E_LineB);
   fprintf(fp, E_LineBA);
   fprintf(fp, E_LineBB);
   fprintf(fp, E_LineC);
   fprintf(fp, E_LineCA);
   fprintf(fp, E_LineCB);

   fprintf(fp, E_LineD);
   fprintf(fp, E_LineDA);
   fprintf(fp, E_LineDB);

   {
      int componentId;
      int phaseId;

      const Snapshot * snapshot = (*snapshotList)[0];

      TrapList * trapList = projectHandle->getTraps(reservoir, snapshot, 0);

      TrapList::iterator trapIter;
      for( trapIter = trapList->begin(); trapIter != trapList->end(); ++trapIter )
      {
         bool flashed;
         const Trap * trap = *trapIter;


         double inputMasses[CBM_NumComponents];
         double outputMasses[CBM_NumPhases][CBM_NumComponents];
         double outputDensities[CBM_NumPhases];
         double outputViscosities[CBM_NumPhases];

         for( componentId = 0; componentId < CBM_NumComponents; ++componentId )
         {
            inputMasses[componentId] = trap->getMass((ComponentId)componentId);
         }

         // flash everything at reservoir conditions
         flashed = pvtFlash::EosPack::getInstance().computeWithLumping(celsius2kelvin(trap->getTemperature()),
            megapascal2pascal(trap->getPressure()),
            inputMasses, outputMasses, outputDensities, outputViscosities);

         if( !flashed ) continue;

         double phaseMasses[CBM_NumPhases];
         for( phaseId = 0; phaseId < CBM_NumPhases; ++phaseId )
         {
            phaseMasses[phaseId] = 0;
            for( componentId = 0; componentId < CBM_NumComponents; ++componentId )
            {
               phaseMasses[phaseId] += outputMasses[phaseId][componentId];
            }

            if( phaseMasses[phaseId] <= 0 ) continue;

            double stInputMasses[CBM_NumComponents];
            double stOutputMasses[CBM_NumPhases][CBM_NumComponents];
            double stOutputDensities[CBM_NumPhases];
            double stOutputViscosities[CBM_NumPhases];

            for( componentId = 0; componentId < CBM_NumComponents; ++componentId )
            {
               stInputMasses[componentId] = outputMasses[phaseId][componentId];
            }

            // flash everything at reservoir conditions
            flashed = pvtFlash::EosPack::getInstance().computeWithLumping(celsius2kelvin(stTemperature),
               megapascal2pascal(stPressure),
               stInputMasses, stOutputMasses, stOutputDensities, stOutputViscosities);

            if( !flashed ) continue;

            char * HCPhase = (char *)(phaseId == PhaseId::OIL ? "OIL" : "GAS");



            double stPhaseMasses[CBM_NumPhases];
            int stPhaseId;
            for( stPhaseId = 0; stPhaseId < CBM_NumPhases; ++stPhaseId )
            {
               stPhaseMasses[stPhaseId] = 0;
               for( componentId = 0; componentId < CBM_NumComponents; ++componentId )
               {
                  stPhaseMasses[stPhaseId] += stOutputMasses[stPhaseId][componentId];
                  assert(stPhaseMasses[stPhaseId] >= stOutputMasses[stPhaseId][componentId]);
               }
            }

            if( debug )
            {
               double stInputMass = 0;

               for( int c = 0; c < CBM_NumComponents; ++c )
               {
                  stInputMass += stInputMasses[c];
               }

               cout << "Trap = " << trap->getId() << ", RC " << HCPhase << " = " << stInputMass
                  << " => ST GAS = " << stPhaseMasses[PhaseId::GAS]
                  << ", ST OIL = " << stPhaseMasses[PhaseId::OIL] << endl;
            }

            double gasVolume = 0;
            if( stOutputDensities[PhaseId::GAS] > 0 )
            {
               gasVolume = stPhaseMasses[PhaseId::GAS] / stOutputDensities[PhaseId::GAS] / 1e6;
            }

            double oilVolume = 0;
            if( stOutputDensities[PhaseId::OIL] > 0 )
            {
               oilVolume = stPhaseMasses[PhaseId::OIL] / stOutputDensities[PhaseId::OIL] / 1e6;
            }

            double API = -1;

            if( stOutputDensities[PhaseId::OIL] > 0 )
            {
               API = 141.5 / (0.001 * stOutputDensities[PhaseId::OIL]) - 131.5;
            }

            char APIString[32], oilDensityString[32], oilGravityString[32],
               gasDensityString[32], gasGravityString[32];

            getFormattedString(stOutputDensities[PhaseId::OIL], "%7.1lf",
               oilDensityString);
            getFormattedString(0.001 * stOutputDensities[PhaseId::OIL], "%7.1lf",
               oilGravityString);
            getFormattedString(stOutputDensities[PhaseId::GAS], "%7.1lf",
               gasDensityString);
            getFormattedString(0.001 * stOutputDensities[PhaseId::GAS], "%7.1lf",
               gasGravityString);
            getFormattedString(API, "%6.1lf", APIString);

            double cvOilVolume = cubicmeters2barrels(oilVolume);
            double cvGasVolume = cubicmeters2standardcubicfeet(gasVolume);

            double trapX, trapY;
            trap->getPosition(trapX, trapY);

            fprintf(fp,
               "%4d|%7.0lf|%7.0lf|%8s|%6.0lf|%7.0lf|%8.0lf|%8.0lf|%7s|%7s|%6s|%5.2lf|%5.2lf|%5.2lf|"
               "%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%7s|"
               "%7s|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|%5.2lf|"
               "%5.2lf|%5.2lf|%5.2lf\n", trap->getId(), trapY, trapX, HCPhase, oilVolume,
               cvOilVolume, gasVolume, cvGasVolume, oilDensityString, oilGravityString,
               APIString,
               percentage(stOutputMasses[PhaseId::OIL][ComponentId::C1], stPhaseMasses[PhaseId::OIL]),
               percentage(stOutputMasses[PhaseId::OIL][ComponentId::C2], stPhaseMasses[PhaseId::OIL]),
               percentage(stOutputMasses[PhaseId::OIL][ComponentId::C3], stPhaseMasses[PhaseId::OIL]),
               percentage(stOutputMasses[PhaseId::OIL][ComponentId::C4], stPhaseMasses[PhaseId::OIL]),
               percentage(stOutputMasses[PhaseId::OIL][ComponentId::C5], stPhaseMasses[PhaseId::OIL]),
               percentage(stOutputMasses[PhaseId::OIL][ComponentId::N2], stPhaseMasses[PhaseId::OIL]),
               percentage(stOutputMasses[PhaseId::OIL][ComponentId::COX], stPhaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::H2S], phaseMasses[PhaseId::OIL]),
               percentage(stOutputMasses[PhaseId::OIL][ComponentId::C6_MINUS_14SAT], stPhaseMasses[PhaseId::OIL]),
               percentage(stOutputMasses[PhaseId::OIL][ComponentId::C6_MINUS_14ARO], stPhaseMasses[PhaseId::OIL]),
               percentage(stOutputMasses[PhaseId::OIL][ComponentId::C15_PLUS_SAT], stPhaseMasses[PhaseId::OIL]),
               percentage(stOutputMasses[PhaseId::OIL][ComponentId::C15_PLUS_ARO], stPhaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::LSC], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::C15_PLUS_AT], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::C15_PLUS_ARO_S], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::C15_PLUS_SAT_S], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::C6_MINUS_14BT], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::C6_MINUS_14DBT], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::C6_MINUS_14BP], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::C6_MINUS_14SAT_S], phaseMasses[PhaseId::OIL]),
               percentage(outputMasses[PhaseId::OIL][ComponentId::C6_MINUS_14ARO_S], phaseMasses[PhaseId::OIL]),
               percentage(stOutputMasses[PhaseId::OIL][ComponentId::RESIN], stPhaseMasses[PhaseId::OIL]),
               percentage(stOutputMasses[PhaseId::OIL][ComponentId::ASPHALTENE], stPhaseMasses[PhaseId::OIL]),
               gasDensityString, gasGravityString,
               percentage(stOutputMasses[PhaseId::GAS][ComponentId::C1], stPhaseMasses[PhaseId::GAS]),
               percentage(stOutputMasses[PhaseId::GAS][ComponentId::C2], stPhaseMasses[PhaseId::GAS]),
               percentage(stOutputMasses[PhaseId::GAS][ComponentId::C3], stPhaseMasses[PhaseId::GAS]),
               percentage(stOutputMasses[PhaseId::GAS][ComponentId::C4], stPhaseMasses[PhaseId::GAS]),
               percentage(stOutputMasses[PhaseId::GAS][ComponentId::C5], stPhaseMasses[PhaseId::GAS]),
               percentage(stOutputMasses[PhaseId::GAS][ComponentId::N2], stPhaseMasses[PhaseId::GAS]),
               percentage(stOutputMasses[PhaseId::GAS][ComponentId::COX], stPhaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::H2S], phaseMasses[PhaseId::GAS]),
               percentage(stOutputMasses[PhaseId::GAS][ComponentId::C6_MINUS_14SAT], stPhaseMasses[PhaseId::GAS]),
               percentage(stOutputMasses[PhaseId::GAS][ComponentId::C6_MINUS_14ARO], stPhaseMasses[PhaseId::GAS]),
               percentage(stOutputMasses[PhaseId::GAS][ComponentId::C15_PLUS_SAT], stPhaseMasses[PhaseId::GAS]),
               percentage(stOutputMasses[PhaseId::GAS][ComponentId::C15_PLUS_ARO], stPhaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::LSC], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::C15_PLUS_AT], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::C15_PLUS_ARO_S], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::C15_PLUS_SAT_S], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::C6_MINUS_14BT], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::C6_MINUS_14DBT], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::C6_MINUS_14BP], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::C6_MINUS_14SAT_S], phaseMasses[PhaseId::GAS]),
               percentage(outputMasses[PhaseId::GAS][ComponentId::C6_MINUS_14ARO_S], phaseMasses[PhaseId::GAS]),
               percentage(stOutputMasses[PhaseId::GAS][ComponentId::RESIN], stPhaseMasses[PhaseId::GAS]),
               percentage(stOutputMasses[PhaseId::GAS][ComponentId::ASPHALTENE], stPhaseMasses[PhaseId::GAS]));
         }
      }
      delete trapList;
   }

   fprintf(fp, E_LineD);
   fprintf(fp, E_LineDA);
   fprintf(fp, E_LineDB);

   fprintf(fp, "\nPVT parameters used for phase properties and volumes are at stock tank conditions.\n");
   fprintf(fp, "Stock tank conditions: %4.1lfC, %lf Pa.\n", stTemperature, stPressure);

   fclose(fp);

   return true;
}

string createLogFileName(ProjectHandle * projectHandle, const Reservoir * reservoir)
{
   string filePath = projectHandle->getName();
   size_t tailPos = filePath.rfind(".project3d");
   if( tailPos != string::npos )
   {
      filePath.erase(tailPos, string::npos);
   }
   filePath += "_";
   filePath += reservoir->getName();
   filePath += ".log";

   return filePath;
}

void showUsage(const char * message)
{
   cerr << endl;
   if( message )
   {
      cerr << argv0 << ": " << message << endl;
   }

   cerr << "Usage: " << argv0
      << " [-verbose] projectfile"
      << endl;
   exit(-1);
}

