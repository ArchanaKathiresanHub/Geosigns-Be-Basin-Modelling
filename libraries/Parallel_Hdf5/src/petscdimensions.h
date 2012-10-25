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

#include "petscda.h"
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
  virtual void         createGlobalArray  (DA &globalArray, const PetscDimensionType &dims) = 0;
  virtual int	       createGlobalVector (DA &globalArray, Vec &globalVec) = 0;
  virtual Petsc_Array* createLocalVector  (DA &globalArray, Vec &globalVec) = 0;

protected:
  void createAll (PetscDimensions *current, DA &globalArray, Vec &globalVec,
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
   Petsc_1D (DA &globalArray, Vec &globalVec, Petsc_Array **localVec, const PetscDimensionType &dims)
   { createAll (this, globalArray, globalVec, localVec, dims); }

   static Petsc_1D* create (Petsc_1D* petscPtr, 
                            DA &globalArray, Vec &globalVec, 
                            Petsc_Array **localVec, const PetscDimensionType &dims)
   {
      return new Petsc_1D (globalArray, globalVec, localVec, dims);
   }

   // public methods
   void createGlobalArray (DA &globalArray, const PetscDimensionType &dims)
   { 
      DACreate1d (PETSC_COMM_WORLD, DA_NONPERIODIC, 
                  dims[0], 1, 1, PETSC_NULL, 
                  &globalArray);
   }

   int createGlobalVector (DA& globalArray, Vec &globalVec)
   {
      return DACreateGlobalVector (globalArray, &globalVec);
   }

   Petsc_Array* createLocalVector (DA &globalArray, Vec &globalVec)
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
   Petsc_2D (DA &globalArray, Vec &globalVec, Petsc_Array **localVec, const PetscDimensionType &dims)
   { createAll (this, globalArray, globalVec, localVec, dims); }

   static Petsc_2D* create (Petsc_2D* petscPtr, 
                            DA &globalArray, Vec &globalVec, 
                            Petsc_Array **localVec, const PetscDimensionType &dims)
   {
      return new Petsc_2D (globalArray, globalVec, localVec, dims);
   }

   // public methods
   void createGlobalArray (DA &globalArray, const PetscDimensionType &dims)
   {
      DACreate2d (PETSC_COMM_WORLD, DA_NONPERIODIC, DA_STENCIL_BOX,
                  dims[0], dims[1], PETSC_DECIDE, PETSC_DECIDE, 
                  1, 1, PETSC_NULL, 
                  PETSC_NULL, &globalArray);
   }

   int createGlobalVector (DA& globalArray, Vec &globalVec)
   {
      return DACreateGlobalVector (globalArray, &globalVec);
   }

   Petsc_Array* createLocalVector (DA &globalArray, Vec &globalVec)
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
   Petsc_3D (DA &globalArray, Vec &globalVec, Petsc_Array **localVec, const PetscDimensionType &dims)
   { createAll (this, globalArray, globalVec, localVec, dims); }

   static Petsc_3D* create (Petsc_3D* petscPtr, 
                            DA &globalArray, Vec &globalVec, 
                            Petsc_Array **localVec, const PetscDimensionType &dims)
   {
      return new Petsc_3D (globalArray, globalVec, localVec, dims);
   }

   // public methods
   void createGlobalArray (DA &globalArray, const PetscDimensionType &dims)
   {
      DACreate3d (PETSC_COMM_WORLD, DA_NONPERIODIC, DA_STENCIL_BOX,
                  dims[0], dims[1], dims[2], PETSC_DECIDE, PETSC_DECIDE, 
                  PETSC_DECIDE, 1, 1, PETSC_NULL, 
                  PETSC_NULL, PETSC_NULL, &globalArray);
   }

   int createGlobalVector (DA& globalArray, Vec &globalVec)
   {
      return DACreateGlobalVector (globalArray, &globalVec);
   }

   Petsc_Array* createLocalVector (DA &globalArray, Vec &globalVec)
   {
      return new PETSC_3D_Array (globalArray, globalVec);
   }
};

#endif
