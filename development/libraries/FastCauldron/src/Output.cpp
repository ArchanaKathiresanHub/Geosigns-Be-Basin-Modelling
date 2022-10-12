//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include "Output.h"
#include "layer_iterators.h"
#include "utils.h"
#include "timefilter.h"

// utilities library
#include "ConstantsNumerical.h"
using Utilities::Numerical::CauldronNoDataValue;

double getPropValOfFirstNodeOnSurface(DM a_DA,
                                      Vec* a_vec,
                                      const SurfacePosition surfacePosition, 
                                      const double factor,
                                      const Boolean2DArray& validNeedle )
{

  //Get the info about our part of the distributed array
  int start[3]  = {0, 0, 0}; 
  int count[3]  = {1, 1, 1}; 

  DMDAGetCorners(a_DA,&start[0],&start[1],&start[2],&count[0],&count[1],&count[2]);

  //*** get the info about the total model
  int size[3] = {0, 0, 0};
  DMDAGetInfo(a_DA, PETSC_IGNORE,&size[0],&size[1],&size[2], 
              PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE, 
              PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE);
  
  double returnValue = CauldronNoDataValue;

  
  //each process gets its first node (but only rank0 process writes the project-file)
  int yCnt = start[1];    
  int xCnt = start[0]; 
  
  if ( size[ 2 ] == 1 )
  {

    PETSC_2D_Array x( a_DA, *a_vec, INSERT_VALUES, true );

    if ( ! validNeedle ( xCnt, yCnt ) || x(yCnt,xCnt) == CauldronNoDataValue) 
    {
       returnValue = CauldronNoDataValue;
    } 
    else 
    {
       returnValue = x(yCnt,xCnt) * factor;
    }

  }
  else
  {
    PETSC_3D_Array x( a_DA, *a_vec, INSERT_VALUES, true );
    
    int zCnt;
    if (surfacePosition == TOPSURFACE) 
    {
       zCnt = start[2]+count[2]-1; 
    } 
    else 
    {
       zCnt = 0;
    }

    if ( ! validNeedle ( xCnt, yCnt ) || x(zCnt,yCnt,xCnt) == CauldronNoDataValue) 
    {
       returnValue = CauldronNoDataValue;
    } 
    else 
    {
       returnValue = x(zCnt,yCnt,xCnt) * factor;
    }
  }      

  return returnValue;
}


void getPropValsOfFirstColumnInLayer(DM a_DA,
                                     Vec* a_vec,
                                     const double factor,
                                     const Boolean2DArray& validNeedle,
    std::list<double>& returnValList)
{
   returnValList.clear();

  //Get the info about our part of the distributed array
  int start[3]  = {0, 0, 0}; 
  int count[3]  = {1, 1, 1}; 

  DMDAGetCorners(a_DA,&start[0],&start[1],&start[2],&count[0],&count[1],&count[2]);

  //*** get the info about the total model
  int size[3] = {0, 0, 0};
  DMDAGetInfo(a_DA, PETSC_IGNORE,&size[0],&size[1],&size[2], 
	    PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE, 
	    PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE);

 
  //each process gets its first node (but only rank0 process writes the project-file)
  int yCnt = start[1];    
  int xCnt = start[0]; 
  
  if ( size[ 2 ] == 1 )
  {

    PETSC_2D_Array x( a_DA, *a_vec, INSERT_VALUES, true );

    if ( ! validNeedle ( xCnt, yCnt ) || x(yCnt,xCnt) == CauldronNoDataValue) 
    {
       returnValList.push_back( CauldronNoDataValue ); 
    } 
    else 
    {
       returnValList.push_back( x(yCnt,xCnt) * factor ); 
    }

  }
  else
  {
    PETSC_3D_Array x( a_DA, *a_vec, INSERT_VALUES, true );
    
    for (int zCnt =  start[2]+count[2]-1 ; zCnt >= start[2]  ; --zCnt) 
    {
       if ( ! validNeedle ( xCnt, yCnt ) || x(zCnt,yCnt,xCnt) == CauldronNoDataValue) 
       {
          returnValList.push_back( CauldronNoDataValue ); 
       } 
       else 
       {
          returnValList.push_back( x(zCnt,yCnt,xCnt) * factor );
       }
    }
  }      
}


