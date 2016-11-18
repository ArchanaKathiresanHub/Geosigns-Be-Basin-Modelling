// Copyright (C) 2010-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "petsc.h"
#include <assert.h>

using namespace std;

#include "RequestDefs.h"

/// Initialize a new request handling phase.
namespace migration
{
   MPI_Datatype ColumnValueType;
   MPI_Datatype ColumnValueArrayType;
   MPI_Datatype ColumnColumnType;
   MPI_Datatype ColumnCompositionType;
   MPI_Datatype ColumnCompositionPositionType;
   MPI_Datatype TrapPropertiesType;
   MPI_Datatype MigrationType;

   MPI_Datatype FormationNodeType;
   MPI_Datatype FormationNodeValueType;
   MPI_Datatype FormationNodeCompositionType;
   MPI_Datatype FormationNodeThreeVectorType;
   MPI_Datatype FormationNodeThreeVectorValueType;

   void InitializeRequestTypes (void)
   {
      Request req;
      ColumnValueRequest cv;
      ColumnValueArrayRequest cva;
      ColumnColumnRequest c2;
      ColumnCompositionRequest cc;
      ColumnCompositionPositionRequest ccp;
      TrapPropertiesRequest tp;
      MigrationRequest m;

      FormationNodeRequest formationNodeRequest;
      FormationNodeValueRequest formationNodeValueRequest;
      FormationNodeCompositionRequest formationNodeCompositionRequest;
      FormationNodeThreeVectorRequest formationNodeThreeVectorRequest;
      FormationNodeThreeVectorValueRequest formationNodeThreeVectorValueRequest;

      int blockSizes[20];
      MPI_Datatype types[20];
      MPI_Aint offsets[20];

      MPI_Aint baseAddress;
      MPI_Aint offsetAddress;

      int baseIndex = 0;
      int index = 0;

      static bool first = true;

      if (!first)  return;
      first = false;

      //*****************************************
      // Request, base class
      MPI_Get_address (&req, &baseAddress);

      MPI_Get_address (&req.valueSpec, &offsetAddress);
      offsets[baseIndex] = offsetAddress - baseAddress;
      types[baseIndex] = MPI_INT;
      blockSizes[baseIndex] = 1;
      ++baseIndex;

      offsets[baseIndex] = (int)((long)(&req.reservoirIndex) - (long)(&req));
      types[baseIndex] = MPI_INT;
      blockSizes[baseIndex] = 1;
      ++baseIndex;

      offsets[baseIndex] = (int)((long)(&req.i) - (long)(&req));
      types[baseIndex] = MPI_INT;
      blockSizes[baseIndex] = 2;
      ++baseIndex;

      //*****************************************
      // ColumnValueRequest
      index = baseIndex;
      MPI_Get_address (&cv, &baseAddress);

      MPI_Get_address (&cv.phase, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_INT;
      blockSizes[index] = 1;
      ++index;

      MPI_Get_address (&cv.value, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = 1;
      ++index;

      MPI_Type_create_struct (index, blockSizes, offsets, types, &ColumnValueType);
      MPI_Type_commit (&ColumnValueType);

      //*****************************************
      // ColumnValueArrayRequest
      index = baseIndex;
      MPI_Get_address (&cva, &baseAddress);

      MPI_Get_address (&cva.phase, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_INT;
      blockSizes[index] = 1;
      ++index;

      MPI_Get_address (&cva.value, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = ColumnValueArraySize;
      ++index;

      MPI_Type_create_struct (index, blockSizes, offsets, types, &ColumnValueArrayType);
      MPI_Type_commit (&ColumnValueArrayType);

      //*****************************************
      // ColumnColumnRequest
      index = baseIndex;
      MPI_Get_address (&c2, &baseAddress);

      MPI_Get_address (&c2.valueI, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_INT;
      blockSizes[index] = 2;
      ++index;

      MPI_Type_create_struct (index, blockSizes, offsets, types, &ColumnColumnType);
      MPI_Type_commit (&ColumnColumnType);

      //*****************************************
      // ColumnCompositionRequest
      index = baseIndex;
      MPI_Get_address (&cc, &baseAddress);

      MPI_Get_address (&cc.phase, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_INT;
      blockSizes[index] = 1;
      ++index;

      MPI_Get_address (&cc.composition.m_components, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = NUM_COMPONENTS;
      ++index;

      MPI_Get_address (&cc.composition.m_density, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = 1;
      ++index;

      MPI_Get_address (&cc.composition.m_viscosity, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = 1;
      ++index;

      MPI_Type_create_struct (index, blockSizes, offsets, types, &ColumnCompositionType);
      MPI_Type_commit (&ColumnCompositionType);

      //*****************************************
      // ColumnCompositionPositionRequest
      index = baseIndex;
      MPI_Get_address(&ccp, &baseAddress);

      MPI_Get_address(&ccp.phase, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_INT;
      blockSizes[index] = 1;
      ++index;

      MPI_Get_address(&ccp.position, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_INT;
      blockSizes[index] = 1;
      ++index;

      MPI_Get_address(&ccp.composition.m_components, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = NUM_COMPONENTS;
      ++index;

      MPI_Get_address(&ccp.composition.m_density, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = 1;
      ++index;

      MPI_Get_address(&ccp.composition.m_viscosity, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = 1;
      ++index;

      MPI_Type_create_struct(index, blockSizes, offsets, types, &ColumnCompositionPositionType);
      MPI_Type_commit(&ColumnCompositionPositionType);

      //*****************************************
      // TrapPropertiesRequest
      index = baseIndex;

      MPI_Get_address (&tp, &baseAddress);

      MPI_Get_address (&tp.id, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_INT;
      blockSizes[index] = 6; // id, spilling, spillid, rank, spillPoint[IJ]
      ++index;

      MPI_Get_address (&tp.capacity, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = 13 + 4 * NumPhases; // capacity, depth, spillDepth, wcSurface, temperature, pressure,
      // permeability, sealPermeability, fracturePressure, netToGross,
      // cep (* 2), criticalTemperature (* 2), interfacialTension (* 2),
      // fractureSealStrength, goc, owc, volume (* 2)
      ++index;

      MPI_Get_address (&tp.composition.m_components, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = NUM_COMPONENTS; // composition
      ++index;

      MPI_Get_address (&tp.composition.m_density, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = 1;
      ++index;

      MPI_Get_address (&tp.composition.m_viscosity, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = 1;
      ++index;

      MPI_Type_create_struct (index, blockSizes, offsets, types, &TrapPropertiesType);
      MPI_Type_commit (&TrapPropertiesType);

      //*****************************************
      // MigrationRequest
      index = 0;

      MPI_Get_address (&m, &baseAddress);

      MPI_Get_address(&m.reservoirIndex, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_INT;
      blockSizes[index] = 1;
      ++index;
	  
      MPI_Get_address (&m.process, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_INT;
      blockSizes[index] = 1;
      ++index;

      MPI_Get_address (&m.source.age, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = 1;
      ++index;

      MPI_Get_address (&m.source.x, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = 2;
      ++index;

      MPI_Get_address (&m.source.trapId, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_INT;
      blockSizes[index] = 1;
      ++index;

      MPI_Get_address (&m.destination.age, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = 1;
      ++index;

      MPI_Get_address (&m.destination.x, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = 2;
      ++index;

      MPI_Get_address (&m.destination.trapId, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_INT;
      blockSizes[index] = 1;
      ++index;

      MPI_Get_address (&m.composition.m_components, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = NUM_COMPONENTS;
      ++index;

      MPI_Get_address (&m.composition.m_density, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = 1;
      ++index;

      MPI_Get_address (&m.composition.m_viscosity, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = 1;
      ++index;

      MPI_Type_create_struct (index, blockSizes, offsets, types, &MigrationType);
      MPI_Type_commit (&MigrationType);

      //*****************************************

      //*****************************************
      // FormationNodeRequest, base class

      baseIndex = 0;
      index = 0;

      MPI_Get_address (&formationNodeRequest, &baseAddress);

      MPI_Get_address (&formationNodeRequest.valueSpec, &offsetAddress);
      offsets[baseIndex] = offsetAddress - baseAddress;
      types[baseIndex] = MPI_INT;
      blockSizes[baseIndex] = 1;
      ++baseIndex;

      MPI_Get_address (&formationNodeRequest.messageId, &offsetAddress);
      offsets[baseIndex] = offsetAddress - baseAddress;
      types[baseIndex] = MPI_INT;
      blockSizes[baseIndex] = 1;
      ++baseIndex;

      MPI_Get_address (&formationNodeRequest.formationIndex, &offsetAddress);
      offsets[baseIndex] = offsetAddress - baseAddress;
      types[baseIndex] = MPI_INT;
      blockSizes[baseIndex] = 1;
      ++baseIndex;

      MPI_Get_address (&formationNodeRequest.i, &offsetAddress);
      offsets[baseIndex] = offsetAddress - baseAddress;
      types[baseIndex] = MPI_INT;
      blockSizes[baseIndex] = 3;
      ++baseIndex;

      //*****************************************
      // FormationNodeValueRequest

      index = baseIndex;
      MPI_Get_address (&formationNodeValueRequest, &baseAddress);

      MPI_Get_address (&formationNodeValueRequest.value, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = 1;
      ++index;

      MPI_Type_create_struct (index, blockSizes, offsets, types, &FormationNodeValueType);
      MPI_Type_commit (&FormationNodeValueType);

      //*****************************************
      // FormationNodeCompositionRequest

      index = baseIndex;
      MPI_Get_address (&formationNodeCompositionRequest, &baseAddress);

      MPI_Get_address (&formationNodeCompositionRequest.phase, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_INT;
      blockSizes[index] = 1;
      ++index;

      MPI_Get_address (&formationNodeCompositionRequest.composition.m_components, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = NUM_COMPONENTS;
      ++index;

      MPI_Get_address (&formationNodeCompositionRequest.composition.m_density, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = 1;
      ++index;

      MPI_Get_address (&formationNodeCompositionRequest.composition.m_viscosity, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = 1;
      ++index;

      MPI_Type_create_struct (index, blockSizes, offsets, types, &FormationNodeCompositionType);
      MPI_Type_commit (&FormationNodeCompositionType);

      //*****************************************
      // FormationNodeThreeVectorRequest

      index = baseIndex;
      MPI_Get_address (&formationNodeThreeVectorRequest, &baseAddress);

      MPI_Get_address (&formationNodeThreeVectorRequest.values, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = 3;
      ++index;

      MPI_Type_create_struct (index, blockSizes, offsets, types, &FormationNodeThreeVectorType);
      MPI_Type_commit (&FormationNodeThreeVectorType);

      //*****************************************
      // FormationNodeThreeVectorValueRequest
      index = baseIndex;
      MPI_Get_address (&formationNodeThreeVectorValueRequest, &baseAddress);

      MPI_Get_address (&formationNodeThreeVectorValueRequest.value, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = 1;
      ++index;

      MPI_Get_address (&formationNodeThreeVectorValueRequest.values, &offsetAddress);
      offsets[index] = offsetAddress - baseAddress;
      types[index] = MPI_DOUBLE;
      blockSizes[index] = 3;
      ++index;

      MPI_Type_create_struct (index, blockSizes, offsets, types, &FormationNodeThreeVectorValueType);
      MPI_Type_commit (&FormationNodeThreeVectorValueType);

      //*****************************************


   }
}

