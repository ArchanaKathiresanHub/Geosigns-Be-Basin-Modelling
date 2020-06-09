//
// Unit tests for FiniteElementMethod::ElementVector 
//

#include "../src/FiniteElementTypes.h"

#include <gtest/gtest.h>

class ElementVectorTest: public ::testing::Test
{
public:
   ElementVectorTest();
   ~ElementVectorTest() {;}

   FiniteElementMethod::ElementVector vec1;
   FiniteElementMethod::ElementVector vec2;
};

ElementVectorTest::ElementVectorTest()
{
   // init test vector 
   for ( int i = 1; i <= 8; ++i ) vec1( i ) = static_cast<double>( i );
   for ( int i = 1; i <= 8; ++i ) vec2( i ) = static_cast<double>( 9-i );
}


TEST_F( ElementVectorTest, methodsTest )
{
   // test access operator
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( vec1(i), static_cast<double>( i ) );

   // test dimension
   EXPECT_EQ( vec1.dimension(), 8 );

   // test operator = 
   FiniteElementMethod::ElementVector vec3 = vec1;
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( vec3(i), vec1(i) );
 
   // test .zero()
   vec3.zero();
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( vec3(i), 0.0 );

   // test .fill()
   vec3.fill( 3.14 );
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( vec3(i), 3.14 );
}
 
TEST_F( ElementVectorTest, maxminTest )
{
   // test v3(i) = maximum( v1(i), val )
   const FiniteElementMethod::ElementVector & vec3 = FiniteElementMethod::maximum( vec1, 9 );
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( vec3(i), 9.0 );

   // test v3(i) = maximum( v1(i), v2(i) )
   const FiniteElementMethod::ElementVector & vec4 = FiniteElementMethod::maximum( vec1, vec2 );
   EXPECT_DOUBLE_EQ( vec4(1), 8.0 );
   EXPECT_DOUBLE_EQ( vec4(2), 7.0 );
   EXPECT_DOUBLE_EQ( vec4(3), 6.0 );
   EXPECT_DOUBLE_EQ( vec4(4), 5.0 );
   EXPECT_DOUBLE_EQ( vec4(5), 5.0 );
   EXPECT_DOUBLE_EQ( vec4(6), 6.0 );
   EXPECT_DOUBLE_EQ( vec4(7), 7.0 );
   EXPECT_DOUBLE_EQ( vec4(8), 8.0 );

   // test v3(i) = minimum( v1(i), v2(i) )
   const FiniteElementMethod::ElementVector & vec5 = FiniteElementMethod::minimum( vec1, vec2 );
   EXPECT_DOUBLE_EQ( vec5(1), 1.0 );
   EXPECT_DOUBLE_EQ( vec5(2), 2.0 );
   EXPECT_DOUBLE_EQ( vec5(3), 3.0 );
   EXPECT_DOUBLE_EQ( vec5(4), 4.0 );
   EXPECT_DOUBLE_EQ( vec5(5), 4.0 );
   EXPECT_DOUBLE_EQ( vec5(6), 3.0 );
   EXPECT_DOUBLE_EQ( vec5(7), 2.0 );
   EXPECT_DOUBLE_EQ( vec5(8), 1.0 );
}

TEST_F( ElementVectorTest, interpolateTest )
{
   // test (1-lambda) * v1 + lambda * v2
   FiniteElementMethod::ElementVector vec3;

   FiniteElementMethod::interpolate( vec1, vec2, 0.2, vec3 );

   EXPECT_NEAR( vec3(1), 2.4, 1e-6 );
   EXPECT_NEAR( vec3(2), 3.0, 1e-6 );
   EXPECT_NEAR( vec3(3), 3.6, 1e-6 );
   EXPECT_NEAR( vec3(4), 4.2, 1e-6 );
   EXPECT_NEAR( vec3(5), 4.8, 1e-6 );
   EXPECT_NEAR( vec3(6), 5.4, 1e-6 );
   EXPECT_NEAR( vec3(7), 6.0, 1e-6 );
   EXPECT_NEAR( vec3(8), 6.6, 1e-6 );
}

TEST_F( ElementVectorTest, linAlgTest )
{
   // test add 
   FiniteElementMethod::ElementVector vec3;

   FiniteElementMethod::add( vec1, vec2, vec3 );
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( vec3(i), vec1(i) + vec2(i) );

   FiniteElementMethod::add( vec1, 2.0, vec2, vec3 );
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( vec3(i), (i + (9-i) * 2.0) );

   FiniteElementMethod::add( 3.0, vec1, 2.0, vec2, vec3 );
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( vec3(i), (3.0 * i + (9-i) * 2.0) );

   FiniteElementMethod::subtract( vec1, vec2, vec3 );
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( vec3(i), vec1(i) - vec2(i) );

   // test v1 += v2
   vec3 = vec1;
   FiniteElementMethod::Increment( vec2, vec3 );
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( vec3(i), vec1(i) + vec2(i) );

   // test v1 += val * v2
   vec3 = vec1;
   FiniteElementMethod::Increment( 2.0, vec2, vec3 );
   for ( int i = 1; i <= 8; ++i ) EXPECT_DOUBLE_EQ( vec3(i), vec1(i) + 2.0 * vec2(i) );

   // test v1 *= val
   vec3 = vec1;
   FiniteElementMethod::scale( vec3, 3.14 );
   for ( int i = 1; i <= 8; ++i ) EXPECT_NEAR( vec3(i), vec1(i) * 3.14, 1e-6 );

   // test v1 dot v2
   EXPECT_NEAR( FiniteElementMethod::innerProduct(vec1, vec2), 120.0, 1e-6 );

   // test |v1|
   EXPECT_NEAR( FiniteElementMethod::length(vec1), 14.2828568570857, 1e-6 );
   
   EXPECT_DOUBLE_EQ( FiniteElementMethod::maxValue( vec1 ), 8.0 );
   vec3 = vec1;
   vec3(8) = -vec3(8);
   EXPECT_DOUBLE_EQ( FiniteElementMethod::maxValue( vec3 ), 7.0 );
}
 
