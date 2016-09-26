#ifndef UTILITIES__ARRAY_DEFINITIONS__H
#define UTILITIES__ARRAY_DEFINITIONS__H

namespace ArrayDefs {

   #define ARRAY_ALIGNMENT 32

#ifdef _MSC_VER
   typedef __declspec(align(ARRAY_ALIGNMENT)) double * Real_ptr;
   typedef __declspec(align(ARRAY_ALIGNMENT)) const double * const ConstReal_ptr;
#elif __INTEL_COMPILER
   // The __restrict__ has to be commented out due to a bug in the Intel compiler
   typedef double * /*__restrict__*/ __attribute__((align_value(ARRAY_ALIGNMENT))) Real_ptr;
   typedef const double * const /*__restrict__*/ __attribute__((align_value(ARRAY_ALIGNMENT))) ConstReal_ptr;
#elif __GNUG__
   typedef double * __restrict__ Real_ptr __attribute__((aligned(ARRAY_ALIGNMENT)));
   typedef const double * const __restrict__ ConstReal_ptr __attribute__((aligned(ARRAY_ALIGNMENT)));
#endif

}

#endif // UTILITIES__ARRAY_DEFINITIONS__H
