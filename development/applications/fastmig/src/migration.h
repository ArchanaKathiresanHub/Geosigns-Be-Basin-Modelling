//
// Copyright (C) 2010-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_H
#define _MIGRATION_H

#include "ComponentManager.h"

#include <vector>
using namespace std;

#define USEOTGC 1

namespace migration
{
   extern int DebugOn;
   extern bool DebugPoint;
   extern unsigned int DebugPointI;
   extern unsigned int DebugPointJ;

   class Trap;
   class Reservoir;
   class Formation;
   class Surface;
   class FormationNode;
   class LocalFormationNode;
   class ProxyFormationNode;
   class FormationNodeArray;
   class Column;
   class LocalColumn;
   class ProxyColumn;
   class Composition;
   class Barrier;
#ifdef USEOTGC
   class OilToGasCracker;
   class Immobiles;
#endif

   class ColumnArray;

   typedef vector<Trap *> TrapVector;
   typedef vector<Trap *>::iterator TrapIterator;
   typedef vector<Trap *>::reverse_iterator TrapReverseIterator;
   typedef vector<const Reservoir *> MigrationReservoirList;

   typedef pair<unsigned int, unsigned int> IndexPair;

   enum ValueSpec
   {
      TOPDEPTH = 0, BOTTOMDEPTH, TOPDEPTHOFFSET, THICKNESS, CAPACITY, NETTOGROSS, POROSITY, IMMOBILESVOLUME, IMMOBILESDENSITY, PERMEABILITY, POROSITYPERCENTAGE,
      FAULTSTATUS, COLUMNSTATUS, PRESSURE, TEMPERATURE, OVERBURDEN, FLUX, FLOW, FLOWDIRECTION, FLOWDIRECTIONIJ,
      ISUNDERSIZED, ISSPILLING, ISSEALING, ISWASTING, PASTEURIZATIONSTATUS, OWCTEMPERATURE, ISTRAPFLAG, SUPPORTEDCHARGEHEIGHT,
      CAPILLARYSEALPRESSURE, CAPILLARYRESERVOIRPRESSURE, CAPILLARYTHRESHOLDPRESSURE,
      ADJACENTCOLUMN, TARGETCOLUMN, SPILLTARGET, TRAPSPILLCOLUMN,
      DRAINAGEAREAID, GLOBALTRAPID, LEAKAGEQUANTITY, FILLDEPTH, PENETRATIONDISTANCE, DIFFUSIONSTARTTIME,
      CHARGEDENSITY, LATERALCHARGEDENSITY, CHARGEQUANTITY, GETCHARGES,
      TARGETFORMATIONNODE, ANALOGFLOWDIRECTION, DEPTH, ISVALID, ISIMPERMEABLE, HASNOTHICKNESS, HASNOWHERETOGO, GOESOUTOFBOUNDS, RESERVOIR,
      ISRESERVOIRVAPOUR, ISRESERVOIRLIQUID, ISENDOFPATH, HEIGHTVAPOUR, HEIGHTLIQUID,
      GETFINITEELEMENTVALUE, GETFINITEELEMENTMINIMUMVALUE, GETFINITEELEMENTGRAD,
      SET /* separator, not used */,
      SETGLOBALTRAPID, SETTOPDEPTH, SETBOTTOMDEPTH, SETFILLDEPTH, SETPENETRATIONDISTANCE, SETDIFFUSIONSTARTTIME, SETCHARGEDENSITY, SETPASTEURIZATIONSTATUS,
      ADDMIGRATED, ADDFLUX,
      INCREASECHARGES, LEAKCHARGES, WASTECHARGES, SPILLCHARGES, ADDCOMPOSITIONTOBEMIGRATED, SETCHARGESTOBEMIGRATED, REGISTER, DEREGISTER,
      ADDTOYOURTRAP, SAVETRAPPROPERTIES,
      RESETPROXY
   };

   extern const char * ValueSpecNames[];

   enum PropertyIndex
   {
      DEPTHPROPERTY, POROSITYPROPERTY, HORIZONTALPERMEABILITYPROPERTY, VERTICALPERMEABILITYPROPERTY, VESPROPERTY, MAXVESPROPERTY, TEMPERATUREPROPERTY, PRESSUREPROPERTY, OVERPRESSUREPROPERTY,
      CAPILLARYENTRYPRESSUREVAPOURPROPERTY, CAPILLARYENTRYPRESSURELIQUIDPROPERTY, LIQUIDDENSITYPROPERTY, VAPOURDENSITYPROPERTY, NUMBEROFPROPERTYINDICES
   };

   extern const char * PropertyIndexNames[];

   enum MigrationProcess
   {
      NOPROCESS, REMIGRATION, REMIGRATIONLEAKAGE, LEAKAGEFROMTRAP, LEAKAGETOTRAP, EXPULSION, ABSORPTION, EXPULSIONLEAKAGE, SPILL, SPILLUPOROUT,
      THROUGHLEAKAGE, DIFFUSION, BIODEGRADATION, OILTOGASCRACKINGLOST, OILTOGASCRACKINGGAINED, NUMBEROFPROCESSES
   };

   extern const char * MigrationProcessNames[];

   /// Column bit values
   enum CacheBit
   {
      TOPDEPTHCACHE = 0, BOTTOMDEPTHCACHE, TOPDEPTHOFFSETCACHE, NETTOGROSSCACHE, POROSITYCACHE, IMMOBILESVOLUMECACHE,
      PERMEABILITYCACHE, FAULTSTATUSCACHE, COLUMNSTATUSCACHE, PASTEURIZATIONSTATUSCACHE,
      BASEADJACENTCOLUMNCACHE, GASADJACENTCOLUMNCACHE = BASEADJACENTCOLUMNCACHE, OILADJACENTCOLUMNCACHE, 
      BASETARGETCOLUMNCACHE, GASTARGETCOLUMNCACHE = BASETARGETCOLUMNCACHE, OILTARGETCOLUMNCACHE,
      BASESEALINGCOLUMNCACHE, GASSEALINGCOLUMNCACHE = BASESEALINGCOLUMNCACHE, OILSEALINGCOLUMNCACHE,
      BASETRAPFLAGCOLUMNCACHE, GASTRAPFLAGCOLUMNCACHE = BASETRAPFLAGCOLUMNCACHE, OILTRAPFLAGCOLUMNCACHE,
      BASEWASTINGCOLUMNCACHE, GASWASTINGCOLUMNCACHE = BASEWASTINGCOLUMNCACHE, OILWASTINGCOLUMNCACHE
   };

   enum FormationNodeCacheBit
   {
      DEPTHCACHE = 0, ISVALIDCACHE, ISIMPERMEABLECACHE, HASNOTHICKNESSCACHE, HASNOWHERETOGOCACHE, GOESOUTOFBOUNDSCACHE,
      HEIGHTVAPOURCACHE, HEIGHTLIQUIDCACHE, TARGETFORMATIONNODECACHE, ANALOGFLOWDIRECTIONCACHE
   };

   const unsigned int BASEADJACENTCOLUMNSET = 0;
   const unsigned int GASADJACENTCOLUMNSET = 0;
   const unsigned int OILADJACENTCOLUMNSET = 1;

   const unsigned int BASETARGETCOLUMNSET = 2;
   const unsigned int GASTARGETCOLUMNSET = 2;
   const unsigned int OILTARGETCOLUMNSET = 3;

   const unsigned int BASESEALINGSET = 4;
   const unsigned int GASSEALINGSET = 4;
   const unsigned int OILSEALINGSET = 5;

   const unsigned int BASEWASTINGSET = 6;
   const unsigned int GASWASTINGSET = 6;
   const unsigned int OILWASTINGSET = 7;

   const unsigned int BASETRAPFLAGSET = 8;
   const unsigned int GASTRAPFLAGSET = 8;
   const unsigned int OILTRAPFLAGSET = 9;

   const int INITIAL = 1;
   const int LEAKED = 2;
   const int WASTED = 4;
   const int SPILLED = 8;

   /// Diffuse C1 to C5 contained in the gas phase of the trap
   const int DiffusionComponentSize = 5;
   const int ColumnValueArraySize = 5;

   const double Sqrt2 = 1.4142135624;

   const int NumberOfNodeCorners = 8;
   const int FirstBottomNodeCorner = 4;
   const int LastBottomNodeCorner = 7;
   const int FirstTopNodeCorner = 0;
   const int LastTopNodeCorner = 3;

   const int NodeCornerOffsets[NumberOfNodeCorners][3] =
      { { 0, 0, 1 }, { 1, 0, 1 }, { 1, 1, 1 }, { 0, 1, 1 }, { 0, 0, 0 }, { 1, 0, 0 }, { 1, 1, 0 }, { 0, 1, 0 } };

   const int NeighbourOffsets2D[8][2] =
   {
      { -1, -1 },
         { -1, 0 },
         { -1, 1 },
         { 0, -1 },
         { 0, 1 },
         { 1, -1 },
         { 1, 0 },
         { 1, 1 }
   };

   const int NumberOfNeighbourOffsets = 26;
   const int NumberOfUpwardNeighbourOffsets = 9;
   const int NumberOfLateralNeighbourOffsets = 8;
   const int NumberOfDownwardNeighbourOffsets = 9;
   const int NumberOfNeighbourOffsetsUsed = NumberOfUpwardNeighbourOffsets + NumberOfLateralNeighbourOffsets;

   const int NeighbourOffsets3D[NumberOfNeighbourOffsets][3] = {
      { 0, 0, 1 }, { -1, 0, 1 }, { 0, -1, 1 }, { 1, 0, 1 }, { 0, 1, 1 }, { -1, -1, 1 }, { -1, 1, 1 }, { 1, -1, 1 }, { 1, 1, 1 },
      { -1, 0, 0 }, { 0, -1, 0 }, { 1, 0, 0 }, { 0, 1, 0 }, { -1, -1, 0 }, { -1, 1, 0 }, { 1, -1, 0 }, { 1, 1, 0 },
      { 0, 0, -1 }, { -1, 0, -1 }, { 0, -1, -1 }, { 1, 0, -1 }, { 0, 1, -1 }, { -1, -1, -1 }, { -1, 1, -1 }, { 1, -1, -1 }, { 1, 1, -1 } };

   const double Offsets[8] =
   {
      Sqrt2,
      1,
      Sqrt2,
      1,
      1,
      Sqrt2,
      1,
      Sqrt2
   };

   const int NeighbourOffsetIndices[3][3] =
   {
      { 0, 1, 2 },
         { 3, -1, 4 },
      { 5, 6, 7 }
   };

   const int DiagonalNeighbourOffsets[4][2] =
   {
      { -1, -1 },
         { -1, 1 },
         { 1, -1 },
         { 1, 1 }
   };

   const int DiagonalNeighbourOffsetIndices[3][3] =
   {
         { 0, -1, 1 },
         { -1, -1, -1 },
         { 2, -1, 3 }
   };

   const int OrthogonalNeighbourOffsets[4][2] =
   {
         { -1, 0 },
         { 0, -1 },
         { 0, 1 },
         { 1, 0 },
   };

   const int OrthogonalNeighbourOffsetIndices[3][3] =
   {
         { -1, 0, -1 },
         { 1, -1, 2 },
         { -1, 3, -1 }
   };

   const int I = 0;
   const int J = 1;

   const int NumNeighbours = 8;
   const int NumDiagonalNeighbours = 4;
   const int NumOrthogonalNeighbours = 4;
   const int MaximumNeighbourOffset = 10; // (MaximumNeighbourOffset - 1) is the maximum number of columns allowed
                                          // between the original and the one under consideration when trying to find
                                          // a suitable column to divert HCs that have ended up in a sealing column.

   const int NoTrapId = -10;
   const int UnknownTrapId = -5;

   const double MinimumMass = 0.5;
   const double MinimumThickness = 0.05;

   const double Fraction2Percentage = 100;
   const double Percentage2Fraction = 0.01;

   const double WaterDensity = 1000;

   const double StockTankPressure = 0.101325;	// Stock tank pressure in MPa
   const double StockTankTemperature = 15.0;		// Stock tank temperature in C

   const double MinColumnHeight = 10.0;

   // virtual reservoir depth at a waste point, a sufficiently large, negative number
   const double WasteDepth = -199999;
   // virtual reservoir depth at a sealing point, a sufficiently large, positive number
   const double SealDepth = 199999;

   // Must correspond to the FaultStatus in (Distributed)DataAccess
   const int NumFaults = 6;

   enum FaultStatus {
      NOFAULT = 0, SEAL, PASS, WASTE, SEALOIL, PASSOIL, NUM_FAULTS = NumFaults
   };

   enum WaterSaturation
   {
      LOW, HIGH
   };

   /// Expulsion directions
   const unsigned int EXPELLEDNONE = 0x0;
   const unsigned int EXPELLEDUPWARD = 0x1;
   const unsigned int EXPELLEDDOWNWARD = 0x2;
   const unsigned int EXPELLEDUPANDDOWNWARD = EXPELLEDUPWARD | EXPELLEDDOWNWARD;

   const unsigned int NumComponents = CBMGenerics::ComponentManager::NumberOfSpecies;
   const unsigned int NumPhases = CBMGenerics::ComponentManager::NumberOfPhases;

   enum PhaseId
   {
      NO_PHASE = -1, FIRST_PHASE = 0, GAS = 0, OIL = 1, LAST_PHASE = NumPhases - 1, NUM_PHASES = NumPhases
   };

   extern const char * ComponentNames[];
#ifdef USEOTGC
   extern const char * ImmobileNames[];
   extern const double ImmobileDensities[];
#endif

   extern const char * ExpulsionDirectionNames[];
   extern const char * TableComponentNames[];
   extern const bool   ComponentsUsed[];
   extern const char * PhaseNames[];
   extern const double DefaultChargeDensities[];
   extern const char * PropertyPrefix;

   extern bool MigrationErrorFound;

   extern const char * BooleanNames[];
   
#ifdef USEOTGC
   enum ImmobilesId
   {
		precoke,
		coke1,
		Hetero1,
		coke2,
		CokeS,
                
      NUM_IMMOBILES = 5
   };

   const unsigned int NumImmobiles = NUM_IMMOBILES;
#endif

   extern ostringstream cerrstrstr;

   template <class T>
   T Square (T x)
   {
      return (x * x);
   }

   template <class T>
   T Min (T x, T y)
   {
      return (x < y ? x : y);
   }

   template <class T>
   T Max (T x, T y)
   {
      return (x > y ? x : y);
   }

   template <class T>
   T Abs (T x)
   {
      return (x >= 0 ? x : -x);
   }

   bool DebugColumn (Column * column);
   
   //Specify the simulation details
   static const std::string simulationModeStr[4] = { "VerticalSecondaryMigration", "HydrodynamicCapillaryPressure", "ReservoirDetection", "InclinedStratigraphy" };
   
}
#endif // _MIGRATION_H
