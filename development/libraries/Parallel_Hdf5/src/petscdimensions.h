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

protected:
  void createAll (PetscDimensions *current, DM &globalArray, Vec &globalVec,
                  Petsc_Array** localVec, const PetscDimensionType &dims) 
  {
     current->createGlobalArray (globalArray, dims);
     current->createGlobalVector (globalArray, globalVec);
     *localVec = current->createLocalVector (globalArray, globalVec);
  }
};

//////////////////////////////////////////
//                                      //
//            Petsc 1D Class            //
//                                      //
//////////////////////////////////////////

class Petsc_1D : public PetscDimensions
{
public:
   // ctor / dtor
   Petsc_1D () : PetscDimensions () {}
   Petsc_1D (DM &globalArray, Vec &globalVec, Petsc_Array **localVec, const PetscDimensionType &dims)
   { createAll (this, globalArray, globalVec, localVec, dims); }

   static Petsc_1D* create (Petsc_1D* petscPtr, 
                            DM &globalArray, Vec &globalVec, 
                            Petsc_Array **localVec, const PetscDimensionType &dims)
   {
      return new Petsc_1D (globalArray, globalVec, localVec, dims);
   }

   // public methods
   void createGlobalArray (DM &globalArray, const PetscDimensionType &dims)
   { 
      DMDACreate1d (PETSC_COMM_WORLD, DMDA_BOUNDARY_NONE, 
                    dims[0], 1, 1, PETSC_NULL, 
                    &globalArray);
   }

   int createGlobalVector (DM& globalArray, Vec &globalVec)
   {
      return DMCreateGlobalVector (globalArray, &globalVec);
   }

   Petsc_Array* createLocalVector (DM &globalArray, Vec &globalVec)
   {
      return new PETSC_1D_Array (globalArray, globalVec);
   }
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
   Petsc_2D (DM &globalArray, Vec &globalVec, Petsc_Array **localVec, const PetscDimensionType &dims)
   { createAll (this, globalArray, globalVec, localVec, dims); }

   static Petsc_2D* create (Petsc_2D* petscPtr, 
                            DM &globalArray, Vec &globalVec, 
                            Petsc_Array **localVec, const PetscDimensionType &dims)
   {
      return new Petsc_2D (globalArray, globalVec, localVec, dims);
   }

   // public methods
   void createGlobalArray (DM &globalArray, const PetscDimensionType &dims)
   {
      DMDACreate2d (PETSC_COMM_WORLD, DMDA_BOUNDARY_NONE, DMDA_BOUNDARY_NONE, DMDA_STENCIL_BOX,
                    dims[0], dims[1], PETSC_DECIDE, PETSC_DECIDE, 
                    1, 1, PETSC_NULL, 
                    PETSC_NULL, &globalArray);
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
   Petsc_3D (DM &globalArray, Vec &globalVec, Petsc_Array **localVec, const PetscDimensionType &dims)
   { createAll (this, globalArray, globalVec, localVec, dims); }

   static Petsc_3D* create (Petsc_3D* petscPtr, 
                            DM &globalArray, Vec &globalVec, 
                            Petsc_Array **localVec, const PetscDimensionType &dims)
   {
      return new Petsc_3D (globalArray, globalVec, localVec, dims);
   }

   // public methods
   void createGlobalArray (DM &globalArray, const PetscDimensionType &dims)
   {
      DMDACreate3d (PETSC_COMM_WORLD, DMDA_BOUNDARY_NONE, DMDA_BOUNDARY_NONE, DMDA_BOUNDARY_NONE, DMDA_STENCIL_BOX,
                    dims[0], dims[1], dims[2], PETSC_DECIDE, PETSC_DECIDE, 
                    PETSC_DECIDE, 1, 1, PETSC_NULL, 
                    PETSC_NULL, PETSC_NULL, &globalArray);
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
