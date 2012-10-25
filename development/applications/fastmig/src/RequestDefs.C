#include "mpi.h"

using namespace std;

#include "RequestDefs.h"

/// Initialize a new request handling phase.
namespace migration
{
MPI_Datatype ColumnValueType;
MPI_Datatype ColumnValueArrayType;
MPI_Datatype ColumnColumnType;
MPI_Datatype ColumnCompositionType;
MPI_Datatype TrapPropertiesType;
MPI_Datatype MigrationType;

void InitializeRequestTypes (void)
{
   Request req;
   ColumnValueRequest cv;
   ColumnColumnRequest c2;
   ColumnCompositionRequest cc;
   TrapPropertiesRequest tp;

   MigrationRequest m;

   int blockSizes[20];
   MPI_Datatype types[20];
   MPI_Aint offsets[20];

   int baseIndex = 0;
   int index = 0;

   static bool first = true;

   if (!first)  return;
   first = false;

   //*****************************************
   // Request, base class

   offsets[baseIndex] = (int) ((long) (&req.valueSpec) - (long) (&req));
   offsets[baseIndex] = 0;
   types[baseIndex] = MPI_INT;
   blockSizes[baseIndex] = 1;
   ++baseIndex;

   offsets[baseIndex] = (int) ((long) (&req.i) - (long) (&req));
   types[baseIndex] = MPI_INT;
   blockSizes[baseIndex] = 2;
   ++baseIndex;

   //*****************************************
   // ColumnValueRequest
   index = baseIndex;
   
   offsets[index] = (int) ((long) (&cv.phase) - (long) (&cv));
   types[index] = MPI_INT;
   blockSizes[index] = 1;
   ++index;

   offsets[index] = (int) ((long) (&cv.value) - (long) (&cv));
   types[index] = MPI_DOUBLE;
   blockSizes[index] = 1;
   ++index;

   offsets[index]= sizeof (cv);
   types[index] = MPI_UB;
   blockSizes[index] = 1;
   ++index;

   MPI_Type_struct (index, blockSizes, offsets, types, &ColumnValueType);
   MPI_Type_commit (&ColumnValueType);

   //*****************************************
   // ColumnValueArrayRequest
   index = baseIndex;
   
   offsets[index] = (int) ((long) (&cv.phase) - (long) (&cv));
   types[index] = MPI_INT;
   blockSizes[index] = 1;
   ++index;

   offsets[index] = (int) ((long) (&cv.value) - (long) (&cv));
   types[index] = MPI_DOUBLE;
   blockSizes[index] = ColumnValueArraySize;
   ++index;

   offsets[index]= sizeof (cv);
   types[index] = MPI_UB;
   blockSizes[index] = 1;
   ++index;

   MPI_Type_struct (index, blockSizes, offsets, types, &ColumnValueArrayType);
   MPI_Type_commit (&ColumnValueArrayType);

   //*****************************************
   // ColumnColumnRequest
   index = baseIndex;

   offsets[index] = (int) ((long) (&c2.valueI) - (long) (&c2));
   types[index] = MPI_INT;
   blockSizes[index] = 2;
   ++index;

   offsets[index]= sizeof (c2);
   types[index] = MPI_UB;
   blockSizes[index] = 1;
   ++index;

   MPI_Type_struct (index, blockSizes, offsets, types, &ColumnColumnType);
   MPI_Type_commit (&ColumnColumnType);

   //*****************************************
   // ColumnCompositionRequest
   index = baseIndex;

   offsets[index] = (int) ((long) (&cc.phase) - (long) (&cc));
   types[index] = MPI_INT;
   blockSizes[index] = 1;
   ++index;

   offsets[index] = (int) ((long) (&cc.composition.m_components) - (long) (&cc));
   types[index] = MPI_DOUBLE;
   blockSizes[index] = NUM_COMPONENTS;
   ++index;

   offsets[index] = (int) ((long) (&cc.composition.m_density) - (long) (&cc));
   types[index] = MPI_DOUBLE;
   blockSizes[index] = 2;
   ++index;

   offsets[index]= sizeof (cc);
   types[index] = MPI_UB;
   blockSizes[index] = 1;
   ++index;

   MPI_Type_struct (index, blockSizes, offsets, types, &ColumnCompositionType);
   MPI_Type_commit (&ColumnCompositionType);

   //*****************************************
   // TrapPropertiesRequest
   index = baseIndex;

   offsets[index] = (int) ((long) (&tp.id) - (long) (&tp));
   types[index] = MPI_INT;
   blockSizes[index] = 6; // id, spilling, spillid, rank, spillPoint[IJ]
   ++index;

   offsets[index] = (int) ((long) (&tp.capacity) - (long) (&tp));
   types[index] = MPI_DOUBLE;
   blockSizes[index] = 13 + 4 * NumPhases; // capacity, depth, spillDepth, wcSurface, temperature, pressure,
                                           // permeability, sealPermeability, fracturePressure, netToGross,
                                           // cep (* 2), criticalTemperature (* 2), interfacialTension (* 2),
                                           // fractureSealStrength, goc, owc, volume (* 2)
   ++index;

   offsets[index] = (int) ((long) (&tp.composition.m_components) - (long) (&tp));
   types[index] = MPI_DOUBLE;
   blockSizes[index] = NUM_COMPONENTS; // composition
   ++index;

   offsets[index]= sizeof (tp);
   types[index] = MPI_UB;
   blockSizes[index] = 1;
   ++index;

   MPI_Type_struct (index, blockSizes, offsets, types, &TrapPropertiesType);
   MPI_Type_commit (&TrapPropertiesType);

   //*****************************************
   // MigrationRequest
   index = 0;

   offsets[index] = (int) ((long) (&m.process) - (long) (&m));
   types[index] = MPI_INT;
   blockSizes[index] = 1;
   ++index;

   offsets[index] = (int) ((long) (&m.source.age) - (long) (&m));
   types[index] = MPI_DOUBLE;
   blockSizes[index] = 1;
   ++index;

   offsets[index] = (int) ((long) (&m.source.x) - (long) (&m));
   types[index] = MPI_DOUBLE;
   blockSizes[index] = 2;
   ++index;

   offsets[index] = (int) ((long) (&m.source.trapId) - (long) (&m));
   types[index] = MPI_INT;
   blockSizes[index] = 1;
   ++index;

   offsets[index] = (int) ((long) (&m.destination.age) - (long) (&m));
   types[index] = MPI_DOUBLE;
   blockSizes[index] = 1;
   ++index;

   offsets[index] = (int) ((long) (&m.destination.x) - (long) (&m));
   types[index] = MPI_DOUBLE;
   blockSizes[index] = 2;
   ++index;

   offsets[index] = (int) ((long) (&m.destination.trapId) - (long) (&m));
   types[index] = MPI_INT;
   blockSizes[index] = 1;
   ++index;

   offsets[index] = (int) ((long) (&m.composition.m_components) - (long) (&m));
   types[index] = MPI_DOUBLE;
   blockSizes[index] = NUM_COMPONENTS;
   ++index;

   offsets[index]= sizeof (m);
   types[index] = MPI_UB;
   blockSizes[index] = 1;
   ++index;

   MPI_Type_struct (index, blockSizes, offsets, types, &MigrationType);
   MPI_Type_commit (&MigrationType);

   //*****************************************
}
}



