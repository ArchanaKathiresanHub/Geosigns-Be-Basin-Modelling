#include "../src/SimdTraits.h"
#include "../src/SimdInstruction.h"
#include <gtest/gtest.h>
#include <stdlib.h>
#include <array>

TEST ( SimdInstrTests, NoSimdTest02 ) {

   typedef Numerics::SimdTraits<Numerics::NO_SIMD> SimdTraits;
   typedef Numerics::SimdInstruction<Numerics::NO_SIMD> SimdInstruction;

   typedef std::array<double, SimdTraits::DoubleStride> DoubleArray;

   double value1 = 2.5;
   double value2 = 3.25;
   double value3 = 1.25;

   double sum = value1 + value2;
   double mul = value1 * value2;
   double muladd = value1 * value2 + value3;

   DoubleArray a;
   SimdTraits::PackedDouble x1 = SimdInstruction::set1 ( value1 );
   SimdTraits::PackedDouble x2 = SimdInstruction::set1 ( value2 );
   SimdTraits::PackedDouble x3 = SimdInstruction::set1 ( value3 );

   SimdTraits::PackedDouble y1 = SimdInstruction::add ( x1, x2 );
   SimdTraits::PackedDouble y2 = SimdInstruction::mul ( x1, x2 );
   SimdTraits::PackedDouble y3 = SimdInstruction::mulAdd ( x1, x2, x3 );

   SimdInstruction::storeu ( &a[0], y1 );
   EXPECT_EQ ( a[0], sum );

   SimdInstruction::storeu ( &a[0], y2 );
   EXPECT_EQ ( a[0], mul );

   SimdInstruction::storeu ( &a[0], y3 );
   EXPECT_EQ ( a[0], muladd );

}

#ifdef __INTEL_COMPILER
#ifdef __SSE__
TEST ( SimdInstrTests, SseTest02 ) {

   static const Numerics::SimdInstructionTechnology SimdUsed = Numerics::SSE;

   typedef Numerics::SimdTraits<SimdUsed> SimdTraits;
   typedef Numerics::SimdInstruction<SimdUsed> SimdInstruction;


   typedef std::array<double, SimdTraits::DoubleStride> DoubleArray;

   double value1 = 2.5;
   double value2 = 3.25;
   double value3 = 1.25;

   double sum = value1 + value2;
   double mul = value1 * value2;
   double muladd = value1 * value2 + value3;

   DoubleArray a;
   SimdTraits::PackedDouble x1 = SimdInstruction::set1 ( value1 );
   SimdTraits::PackedDouble x2 = SimdInstruction::set1 ( value2 );
   SimdTraits::PackedDouble x3 = SimdInstruction::set1 ( value3 );

   SimdTraits::PackedDouble y1 = SimdInstruction::add ( x1, x2 );
   SimdTraits::PackedDouble y2 = SimdInstruction::mul ( x1, x2 );
   SimdTraits::PackedDouble y3 = SimdInstruction::mulAdd ( x1, x2, x3 );

   SimdInstruction::storeu ( &a[0], y1 );
   EXPECT_EQ ( a[0], sum );
   EXPECT_EQ ( a[1], sum );

   SimdInstruction::storeu ( &a[0], y2 );
   EXPECT_EQ ( a[0], mul );
   EXPECT_EQ ( a[1], mul );

   SimdInstruction::storeu ( &a[0], y3 );
   EXPECT_EQ ( a[0], muladd );
   EXPECT_EQ ( a[1], muladd );
}
#endif

#ifdef __AVX__
TEST ( SimdInstrTests, AvxTest02 ) {

   static const Numerics::SimdInstructionTechnology SimdUsed = Numerics::AVX;

   typedef Numerics::SimdTraits<SimdUsed> SimdTraits;
   typedef Numerics::SimdInstruction<SimdUsed> SimdInstruction;


   typedef array<double, SimdTraits::DoubleStride> DoubleArray;

   double value1 = 2.5;
   double value2 = 3.25;
   double value3 = 1.25;

   double sum = value1 + value2;
   double mul = value1 * value2;
   double muladd = value1 * value2 + value3;

   DoubleArray a;
   SimdTraits::PackedDouble x1 = SimdInstruction::set1 ( value1 );
   SimdTraits::PackedDouble x2 = SimdInstruction::set1 ( value2 );
   SimdTraits::PackedDouble x3 = SimdInstruction::set1 ( value3 );

   SimdTraits::PackedDouble y1 = SimdInstruction::add ( x1, x2 );
   SimdTraits::PackedDouble y2 = SimdInstruction::mul ( x1, x2 );
   SimdTraits::PackedDouble y3 = SimdInstruction::mulAdd ( x1, x2, x3 );

   SimdInstruction::storeu ( &a[0], y1 );
   EXPECT_EQ ( a[0], sum );
   EXPECT_EQ ( a[1], sum );
   EXPECT_EQ ( a[2], sum );
   EXPECT_EQ ( a[3], sum );

   SimdInstruction::storeu ( &a[0], y2 );
   EXPECT_EQ ( a[0], mul );
   EXPECT_EQ ( a[1], mul );
   EXPECT_EQ ( a[2], mul );
   EXPECT_EQ ( a[3], mul );

   SimdInstruction::storeu ( &a[0], y3 );
   EXPECT_EQ ( a[0], muladd );
   EXPECT_EQ ( a[1], muladd );
   EXPECT_EQ ( a[2], muladd );
   EXPECT_EQ ( a[3], muladd );
}
#endif
#endif
