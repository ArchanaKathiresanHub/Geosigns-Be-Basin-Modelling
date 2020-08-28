//
// Classes to handle Petsc Vectors and Local arrays
// of different dimensions
//

#ifndef __petscdimensions__
#define __petscdimensions__

//////////////////////////////////////////
//                                      //
// Petsc Dimensions Abstract Base Class //
//                                      //
//////////////////////////////////////////

#include "petscdmda.h"
#include "PetscVectors.h"
#include "h5_types.h"

typedef H5_Dimensions<hsize_t> PetscDimensionType;

class PetscDimensions
{
public:
  // ctor / dtor
  PetscDimensions () {}
  virtual ~PetscDimensions () {}
  
  // public methods
  virtual void         createGlobalArray  (DM &globalArray, const PetscDimensionType &dims) = 0;
  virtual int	       createGlobalVector (DM &globalArray, Vec &globalVec) = 0;
  virtual Petsc_Array* createLocalVector  (DM &globalArray, Vec &globalVec) = 0;
};


//////////////////////////////////////////
//                                      //
//            Petsc 2D Class            //
//                                      //
//////////////////////////////////////////

class Petsc_2D : public PetscDimensions
{
public:
   // ctor / dtor
   Petsc_2D () : PetscDimensions () {}

   // public methods
   void createGlobalArray (DM &globalArray, const PetscDimensionType &dims)
   {
       PetscErrorCode ierr = DMDACreate2d(PETSC_COMM_WORLD, DM_BOUNDARY_NONE, DM_BOUNDARY_NONE, DMDA_STENCIL_BOX,
           (PetscInt)(dims[0]), (PetscInt)(dims[1]), PETSC_DECIDE, PETSC_DECIDE,
           1, 1, PETSC_IGNORE,
           PETSC_IGNORE, &globalArray);
       ierr = DMSetFromOptions(globalArray);
       ierr = DMSetUp(globalArray);
   }

   int createGlobalVector (DM& globalArray, Vec &globalVec)
   {
      return DMCreateGlobalVector (globalArray, &globalVec);
   }

   Petsc_Array* createLocalVector (DM &globalArray, Vec &globalVec)
   {
      return new PETSC_2D_Array (globalArray, globalVec);
   }
};

//////////////////////////////////////////
//                                      //
//            Petsc 3D Class            //
//                                      //
//////////////////////////////////////////

class Petsc_3D : public PetscDimensions
{
public:
   // ctor / dtor
   Petsc_3D () : PetscDimensions () {}

   // public methods
   void createGlobalArray (DM &globalArray, const PetscDimensionType &dims)
   {
       PetscErrorCode err = DMDACreate3d (PETSC_COMM_WORLD, DM_BOUNDARY_NONE, DM_BOUNDARY_NONE, DM_BOUNDARY_NONE, DMDA_STENCIL_BOX,
                    (PetscInt)dims[0], (PetscInt)dims[1], (PetscInt)dims[2], PETSC_DECIDE, PETSC_DECIDE,
                    PETSC_DECIDE, 1, 1, PETSC_IGNORE, 
                    PETSC_IGNORE, PETSC_IGNORE, &globalArray);
      err = DMSetFromOptions(globalArray);
      err = DMSetUp(globalArray);
   }

   int createGlobalVector (DM& globalArray, Vec &globalVec)
   {
      return DMCreateGlobalVector (globalArray, &globalVec);
   }

   Petsc_Array* createLocalVector (DM &globalArray, Vec &globalVec)
   {
      return new PETSC_3D_Array (globalArray, globalVec);
   }
};

#endif
