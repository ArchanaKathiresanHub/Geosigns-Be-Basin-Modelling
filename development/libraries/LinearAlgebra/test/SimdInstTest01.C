#include "../src/SimdTraits.h"
#include "../src/SimdInstruction.h"
#include <gtest/gtest.h>
#include <stdlib.h>

#include <iostream>
using namespace std;

TEST ( SimdInstrTests, NoSimdTest01 ) {

   typedef Numerics::SimdTraits<Numerics::NO_SIMD> SimdTraits;
   typedef Numerics::SimdInstruction<Numerics::NO_SIMD> SimdInstruction;

   double value = 2.5;
   double* a1 = SimdInstruction::allocate ( SimdTraits::DoubleStride );
   double* a2 = new double [ SimdTraits::DoubleStride ];
   SimdTraits::PackedDouble b = SimdInstruction::set1 ( value );

   SimdInstruction::store ( a1, b );
   SimdInstruction::storeu ( a2, b );

   EXPECT_EQ ( a1[0], value );
   EXPECT_EQ ( a2[0], value );

   SimdInstruction::free ( a1 );
   delete [] a2;

}

#ifdef __SSE__
TEST ( SimdInstrTests, SseTest01 ) {

   static const Numerics::SimdInstructionTechnology SimdUsed = Numerics::SSE;

   typedef Numerics::SimdTraits<SimdUsed> SimdTraits;
   typedef Numerics::SimdInstruction<SimdUsed> SimdInstruction;

   double value = 2.5;
   double* a1 = SimdInstruction::allocate ( SimdTraits::DoubleStride );
   double* a2 = new double [ SimdTraits::DoubleStride ];
   SimdTraits::PackedDouble b = SimdInstruction::set1 ( value );


   // Check for memory alignment.
   EXPECT_EQ ( (long long)(a1) % SimdInstruction::Alignment, 0 );

   SimdInstruction::store ( a1, b );
   SimdInstruction::storeu ( a2, b );

   // Check that value retrieved is that same as the one set for the aligned storage.
   EXPECT_EQ ( a1[0], value );
   EXPECT_EQ ( a1[1], value );

   // Check that value retrieved is that same as the one set for the un-aligned storage.
   EXPECT_EQ ( a2[0], value );
   EXPECT_EQ ( a2[1], value );

   SimdInstruction::free ( a1 );
   delete [] a2;

}
#endif

#ifdef __AVX__
TEST ( SimdInstrTests, AvxTest01 ) {

   static const Numerics::SimdInstructionTechnology SimdUsed = Numerics::AVX;

   typedef Numerics::SimdTraits<SimdUsed> SimdTraits;
   typedef Numerics::SimdInstruction<SimdUsed> SimdInstruction;

   double value = 2.5;
   double* a1 = SimdInstruction::allocate ( SimdTraits::DoubleStride );
   double* a2 = new double [ SimdTraits::DoubleStride ];
   SimdTraits::PackedDouble b = SimdInstruction::set1 ( value );

   // Check for memory alignment.
   EXPECT_EQ ( (long long)(a1) % SimdInstruction::Alignment, 0 );

   SimdInstruction::store ( a1, b );
   SimdInstruction::storeu ( a2, b );

   // Check that value retrieved is that same as the one set for the aligned storage.
   EXPECT_EQ ( a1[0], value );
   EXPECT_EQ ( a1[1], value );
   EXPECT_EQ ( a1[2], value );
   EXPECT_EQ ( a1[3], value );

   // Check that value retrieved is that same as the one set for the un-aligned storage.
   EXPECT_EQ ( a2[0], value );
   EXPECT_EQ ( a2[1], value );
   EXPECT_EQ ( a2[2], value );
   EXPECT_EQ ( a2[3], value );

   SimdInstruction::free ( a1 );
   delete [] a2;
}
#endif
