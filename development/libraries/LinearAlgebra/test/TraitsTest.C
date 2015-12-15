#include "../src/SimdTraits.h"
#include <gtest/gtest.h>

TEST ( SimdTraitTests, NoSimdTest ) {
   EXPECT_EQ ( true, Numerics::SimdTraits<Numerics::NO_SIMD>::Alignment == sizeof (double));
   EXPECT_EQ ( true, Numerics::SimdTraits<Numerics::NO_SIMD>::DoubleStride == 1 );
   EXPECT_EQ ( true, sizeof ( Numerics::SimdTraits<Numerics::NO_SIMD>::PackedDouble ) == sizeof (double));
}

#ifdef __SSE__
TEST ( SimdTraitTests, SseTest ) {
   EXPECT_EQ ( true, Numerics::SimdTraits<Numerics::SSE>::Alignment == 2 * sizeof (double));
   EXPECT_EQ ( true, Numerics::SimdTraits<Numerics::SSE>::DoubleStride == 2 );
   EXPECT_EQ ( true, sizeof ( Numerics::SimdTraits<Numerics::SSE>::PackedDouble ) == 2 * sizeof (double));
}
#endif

#ifdef __AVX__
TEST ( SimdTraitTests, AvxTest ) {
   EXPECT_EQ ( true, Numerics::SimdTraits<Numerics::AVX>::Alignment == 4 * sizeof (double));
   EXPECT_EQ ( true, Numerics::SimdTraits<Numerics::AVX>::DoubleStride == 4 );
   EXPECT_EQ ( true, sizeof ( Numerics::SimdTraits<Numerics::AVX>::PackedDouble ) == 4 * sizeof (double));
}
#endif
