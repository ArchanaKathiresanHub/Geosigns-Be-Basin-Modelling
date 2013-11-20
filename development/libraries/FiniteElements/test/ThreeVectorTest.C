//
// Unit tests for FiniteElementMethod::ThreeVector
//

#include "../src/FiniteElementTypes.h"

#include <gtest/gtest.h>

class ThreeVectorTest: public ::testing::Test
{
public:
   ThreeVectorTest();
   ~ThreeVectorTest() {;}

   FiniteElementMethod::ThreeVector vec1;
};

ThreeVectorTest::ThreeVectorTest()
{
   // init test matrix
   vec1( 1 ) = 16;
   vec1( 2 ) = 10;
   vec1( 3 ) = 70;
}


TEST_F( ThreeVectorTest, methodsTest )
{
   // test access operator
   EXPECT_DOUBLE_EQ( vec1(1), 16.0 );
   EXPECT_DOUBLE_EQ( vec1(2), 10.0 );
   EXPECT_DOUBLE_EQ( vec1(3), 70.0 );

   // test dimension
   EXPECT_EQ( vec1.dimension(), 3 );

   // test operator = 
   FiniteElementMethod::ThreeVector vec2 = vec1;
   for ( int i = 1; i <=3; ++i ) EXPECT_DOUBLE_EQ( vec2(i), vec1(i) );
   
   // test .zero()
   vec2.zero();
   for ( int i = 1; i <=3; ++i ) EXPECT_DOUBLE_EQ( vec2(i), 0.0 );

   // test .fill()
   vec2.fill( 3.14 );
   for ( int i = 1; i <=3; ++i ) EXPECT_DOUBLE_EQ( vec2(i), 3.14 );

   // test operator +=
   FiniteElementMethod::ThreeVector vec3 = vec1;
   vec3 += vec2;
   for ( int i = 1; i <=3; ++i ) EXPECT_DOUBLE_EQ( vec3(i), vec1(i) + 3.14 );

   // test operator -=
   vec3 -= vec2;
   for ( int i = 1; i <=3; ++i ) EXPECT_DOUBLE_EQ( vec3(i), vec1(i) );

   // test operator +=
   vec3 += 3.14;
   for ( int i = 1; i <=3; ++i ) EXPECT_DOUBLE_EQ( vec3(i), vec1(i) + 3.14 );
   
   // test operator -=
   vec3 -= 3.14;
   for ( int i = 1; i <=3; ++i ) EXPECT_DOUBLE_EQ( vec3(i), vec1(i) );

   // test operator *=
   vec3 *= 3.14;
   for ( int i = 1; i <=3; ++i ) EXPECT_DOUBLE_EQ( vec3(i), vec1(i) * 3.14 );

   // test operator /=
   vec3 /= 3.14;
   for ( int i = 1; i <=3; ++i ) EXPECT_NEAR( vec3(i), vec1(i), 1e-8 );

   // test .length()
   EXPECT_NEAR( length( vec1 ), 72.4982758415674, 1e-6 );

   // test .scale()
   vec3 = vec1;
   FiniteElementMethod::scale( vec3, 3.14 );
   for ( int i = 1; i <=3; ++i ) EXPECT_DOUBLE_EQ( vec3(i), vec1(i) * 3.14 );

   // test .normalise()
   vec3 = vec1;
   FiniteElementMethod::normalise( vec3 );
   for ( int i = 1; i <=3; ++i ) EXPECT_NEAR( vec3(i), vec1(i) / 72.4982758415674, 1e-6 );

   // test .innerProduct()
   EXPECT_NEAR( FiniteElementMethod::innerProduct( vec1, vec3 ), 72.4982758415674, 1e-6 );
}

TEST_F( ThreeVectorTest, functionsTest )
{
   FiniteElementMethod::ThreeVector vec2 = vec1;
   vec2(3) = -vec1(3);

   // test maximum
   EXPECT_DOUBLE_EQ( FiniteElementMethod::maximum( vec1 ), 70.0 );
   EXPECT_DOUBLE_EQ( FiniteElementMethod::maximum( vec2 ), 16.0 );
   
   // test maximumAbs
   EXPECT_DOUBLE_EQ( FiniteElementMethod::maximumAbs( vec2 ), 70 );

   // test operator +
   const FiniteElementMethod::ThreeVector & vec3 = vec1 + vec2;
   for ( int i = 1; i <=3; ++i ) EXPECT_DOUBLE_EQ( vec3(i), vec1(i) + vec2(i) );

   // test operator -
   const FiniteElementMethod::ThreeVector & vec4 = vec1 + vec2;
   for ( int i = 1; i <=3; ++i ) EXPECT_DOUBLE_EQ( vec4(i), vec1(i) + vec2(i) );

   // test operator *
   const FiniteElementMethod::ThreeVector & vec5 = 3.14 * vec1;
   for ( int i = 1; i <=3; ++i ) EXPECT_DOUBLE_EQ( vec5(i), vec1(i) * 3.14 );

   const FiniteElementMethod::ThreeVector & vec6 = vec1 * 3.14;
   for ( int i = 1; i <=3; ++i ) EXPECT_DOUBLE_EQ( vec6(i), vec1(i) * 3.14 );

   // test operator / 
   const FiniteElementMethod::ThreeVector & vec7 = vec1 / 3.14;
   for ( int i = 1; i <=3; ++i ) EXPECT_NEAR( vec7(i), vec1(i) / 3.14, 1e-6 );
}
 
