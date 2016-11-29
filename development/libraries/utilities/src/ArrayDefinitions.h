//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef UTILITIES__ARRAY_DEFINITIONS__H
#define UTILITIES__ARRAY_DEFINITIONS__H

namespace ArrayDefs
{
   /// \def ARRAY_ALIGNMENT
   /// \brief The alignment, in bytes, required by arrays of the types below (Real_ptr and ConstReal_ptr)
  #define ARRAY_ALIGNMENT 32

#ifdef _MSC_VER
   typedef __declspec(align(ARRAY_ALIGNMENT)) int * Int_ptr;
   typedef __declspec(align(ARRAY_ALIGNMENT)) const int * const ConstInt_ptr;
   typedef __declspec(align(ARRAY_ALIGNMENT)) double * Real_ptr;
   typedef __declspec(align(ARRAY_ALIGNMENT)) const double * const ConstReal_ptr;
#elif __INTEL_COMPILER
   // The __restrict__ has to be commented out due to a bug in the Intel compiler
   typedef int * /*__restrict__*/ __attribute__((align_value(ARRAY_ALIGNMENT))) Int_ptr;
   typedef const int * const /*__restrict__*/ __attribute__((align_value(ARRAY_ALIGNMENT))) ConstInt_ptr;
   typedef double * /*__restrict__*/ __attribute__((align_value(ARRAY_ALIGNMENT))) Real_ptr;
   typedef const double * const /*__restrict__*/ __attribute__((align_value(ARRAY_ALIGNMENT))) ConstReal_ptr;
#elif __GNUG__
   typedef int * __restrict__ Int_ptr /* __attribute__((aligned(ARRAY_ALIGNMENT))) */;
   typedef const int * const __restrict__ ConstInt_ptr /* __attribute__((aligned(ARRAY_ALIGNMENT))) */;
   typedef double * __restrict__ Real_ptr /* __attribute__((aligned(ARRAY_ALIGNMENT))) */;
   typedef const double * const __restrict__ ConstReal_ptr /* __attribute__((aligned(ARRAY_ALIGNMENT))) */;
#endif

}

#endif // UTILITIES__ARRAY_DEFINITIONS__H
