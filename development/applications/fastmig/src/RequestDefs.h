#ifndef _MIGRATION_REQUESTDEFS_H_
#define _MIGRATION_REQUESTDEFS_H_

#include "mpi.h"

#include "Composition.h"
#include "migration.h"


namespace migration
{
   void InitializeRequestTypes (void);

   struct MigrationHub
   {
      inline bool operator== (const MigrationHub &) const;
      double age;

      double x;
      double y;

      int trapId;
   };

   bool MigrationHub::operator== (const MigrationHub & mh) const
   {
      return (mh.age == age &&
	    mh.x == x &&
	    mh.y == y &&
	    mh.trapId == trapId);
   }

#define USECONSTRUCTOR 1
   struct MigrationRequest
   {
#ifdef USECONSTRUCTOR
      inline MigrationRequest (void);
#endif
      inline bool operator== (const MigrationRequest &) const;

      MigrationProcess process;
      MigrationHub source;
      MigrationHub destination;
      Composition composition;
   };

   typedef vector<MigrationRequest> MigrationRequestVector;
   typedef vector<MigrationRequest>::iterator MigrationRequestIterator;


#ifdef USECONSTRUCTOR
   MigrationRequest::MigrationRequest (void): process (NOPROCESS)
   {
   }
#endif

   bool MigrationRequest::operator== (const MigrationRequest & mr) const
   {
      return (process == mr.process &&
	    source == mr.source &&
	    destination == mr.destination);
   }


   struct Request
   {
      ValueSpec valueSpec;

      int i;
      int j;
   };

   struct ColumnValueRequest : public Request
   {
      int phase;
      double value;
   };

   struct ColumnValueArrayRequest : public Request
   {
      int phase;
      double value[ColumnValueArraySize];
   };

   struct ColumnColumnRequest : public Request
   {
      int valueI;
      int valueJ;
   };

   struct ColumnColumnValueRequest : public Request
   {
      int valueI;
      int valueJ;

      int phase;
      double value;
   };

   struct ColumnCompositionRequest : public Request
   {
      int phase;
      Composition composition;
   };

   struct TrapPropertiesRequest : public Request
   {
      inline TrapPropertiesRequest (void);
      int id;
      int spilling;
      int spillid;
      int rank;
      int spillPointI;
      int spillPointJ;
      double capacity;
      double depth;
      double spillDepth;
      double wcSurface;
      double temperature;
      double pressure;
      double permeability;
      double sealPermeability;
      double fracturePressure;
      double netToGross;
      double cep[NumPhases];
      double criticalTemperature[NumPhases];
      double interfacialTension[NumPhases];
      double fractureSealStrength;
      double goc;
      double owc;
      double volume[NumPhases];
      Composition composition;
   };

   TrapPropertiesRequest::TrapPropertiesRequest (void): id (-1), rank (-1), capacity (0)
   {
      volume[GAS] = 0;
      volume[OIL] = 0;
   }

   extern MPI_Datatype ColumnValueType;
   extern MPI_Datatype ColumnValueArrayType;
   extern MPI_Datatype ColumnColumnType;
   extern MPI_Datatype ColumnCompositionType;
   extern MPI_Datatype TrapPropertiesType;
   extern MPI_Datatype MigrationType;

}

#endif
