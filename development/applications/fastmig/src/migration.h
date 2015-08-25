#ifndef _MIGRATION_H
#define _MIGRATION_H

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

   enum ValueSpec
   {
      TOPDEPTH = 0, BOTTOMDEPTH, TOPDEPTHOFFSET, THICKNESS, CAPACITY, NETTOGROSS, POROSITY, IMMOBILESVOLUME, IMMOBILESDENSITY, PERMEABILITY, POROSITYPERCENTAGE,
      FAULTSTATUS, COLUMNSTATUS, PRESSURE, TEMPERATURE, OVERBURDEN, FLUX, FLOW, FLOWDIRECTION,
      ISUNDERSIZED, ISSPILLING, ISSEALING, ISWASTING,
      ADJACENTCOLUMN, TARGETCOLUMN, SPILLTARGET, TRAPSPILLCOLUMN,
      DRAINAGEAREAID, GLOBALTRAPID, LEAKAGEQUANTITY, FILLDEPTH, PENETRATIONDISTANCE, DIFFUSIONSTARTTIME,
      CHARGEDENSITY, LATERALCHARGEDENSITY, CHARGEQUANTITY,
      SET /* separator, not used */,
      SETGLOBALTRAPID, SETTOPDEPTH, SETBOTTOMDEPTH, SETFILLDEPTH, SETPENETRATIONDISTANCE, SETDIFFUSIONSTARTTIME, SETCHARGEDENSITY,
      ADDMIGRATED, ADDFLUX,
      INCREASECHARGES, LEAKCHARGES, WASTECHARGES, SPILLCHARGES, SETCHARGESTOBEMIGRATED, REGISTER, DEREGISTER,
      ADDTOYOURTRAP, SAVETRAPPROPERTIES,
      RESETPROXY
   };

   enum MigrationProcess
   {
      NOPROCESS, REMIGRATION, REMIGRATIONLEAKAGE, LEAKAGEFROMTRAP, LEAKAGETOTRAP, EXPULSION, ABSORPTION, EXPULSIONLEAKAGE, SPILL, SPILLUPOROUT, THROUGHLEAKAGE, DIFFUSION, BIODEGRADATION, OILTOGASCRACKINGLOST, OILTOGASCRACKINGGAINED, NUMBEROFPROCESSES
   };

   extern const char * MigrationProcessNames[];

   /// Column bit values
   enum CacheBit
   {
      TOPDEPTHCACHE = 0, BOTTOMDEPTHCACHE, TOPDEPTHOFFSETCACHE, NETTOGROSSCACHE, POROSITYCACHE, IMMOBILESVOLUMECACHE,
      PERMEABILITYCACHE, FAULTSTATUSCACHE, COLUMNSTATUSCACHE,
      BASEADJACENTCOLUMNCACHE, GASADJACENTCOLUMNCACHE = BASEADJACENTCOLUMNCACHE, OILADJACENTCOLUMNCACHE, 
      BASETARGETCOLUMNCACHE, GASTARGETCOLUMNCACHE = BASETARGETCOLUMNCACHE, OILTARGETCOLUMNCACHE,
      BASESEALINGCOLUMNCACHE, GASSEALINGCOLUMNCACHE = BASESEALINGCOLUMNCACHE, OILSEALINGCOLUMNCACHE,
      BASEWASTINGCOLUMNCACHE, GASWASTINGCOLUMNCACHE = BASEWASTINGCOLUMNCACHE, OILWASTINGCOLUMNCACHE
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

   const int INITIAL = 1;
   const int LEAKED = 2;
   const int WASTED = 4;
   const int SPILLED = 8;

   const int DiffusionComponentSize = 3;

   const int ColumnValueArraySize = 5;

   const double Sqrt2 = 1.4142135624;

   const int NeighbourOffsets[8][2] =
   {
      { -1, -1 },
      { -1,  0 },
      { -1,  1 },
      {  0, -1 },
      {  0,  1 },
      {  1, -1 },
      {  1,  0 },
      {  1,  1 }
   };

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
      { 3,-1, 4 },
      { 5, 6, 7 }
   };

   const int DiagonalNeighbourOffsets[4][2] =
   {
      { -1, -1 },
      { -1,  1 },
      {  1, -1 },
      {  1,  1 }
   };

   const int DiagonalNeighbourOffsetIndices[3][3] =
   {
      { 0,-1, 1 },
      {-1,-1,-1 },
      { 2,-1, 3 }
   };

   const int OrthogonalNeighbourOffsets[4][2] =
   {
      { -1,  0 },
      {  0, -1 },
      {  0,  1 },
      {  1,  0 },
   };

   const int OrthogonalNeighbourOffsetIndices[3][3] =
   {
      {-1, 0,-1 },
      { 1,-1, 2 },
      {-1, 3,-1 }
   };

   const int I = 0;
   const int J = 1;
   const int NumNeighbours = 8;
   const int NumDiagonalNeighbours = 4;
   const int NumOrthogonalNeighbours = 4;

   const int NoTrapId = -10;
   const int UnknownTrapId = -5;

   const double MinimumMass = .5;
   const double MinimumThickness = 0.05;

   const double Fraction2Percentage = 100;
   const double Percentage2Fraction = 0.01;

   const double WaterDensity = 1000;

   const double StockTankPressure = 0.101325;		// Stock tank pressure in MPa
   const double StockTankTemperature = 15.0;		// Stock tank temperature in C

   // virtual reservoir depth at a waste point, a sufficiently large, negative number
   const double WasteDepth = -199999;
   // virtual reservoir depth at a sealing point, a sufficiently large, positive number
   const double SealDepth = 199999;

   // Must correspond to the FaultStatus in (Distributed)DataAccess
   const int NumFaults = 6;

   enum FaultStatus {
      NOFAULT = 0, SEAL, PASS, WASTE, SEALOIL, PASSOIL, NUM_FAULTS = NumFaults
   };

   /// Expulsion directions
   const unsigned int EXPELLEDNONE =          0x0;
   const unsigned int EXPELLEDUPWARD =        0x1;
   const unsigned int EXPELLEDDOWNWARD =      0x2;
   const unsigned int EXPELLEDUPANDDOWNWARD = EXPELLEDUPWARD | EXPELLEDDOWNWARD;

   const unsigned int NumComponents = 23; //13;
   const unsigned int NumPhases = 2;

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
   extern const bool ComponentsUsed[];
   extern const char * PhaseNames[];
   extern const double DefaultChargeDensities[];
   extern const char * PropertyPrefi;
   
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
}
#endif // _MIGRATION_H
