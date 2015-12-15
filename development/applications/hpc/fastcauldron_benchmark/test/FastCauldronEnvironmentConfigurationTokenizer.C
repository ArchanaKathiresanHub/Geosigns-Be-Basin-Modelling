#include "FastCauldronEnvironmentConfigurationTokenizer.h"

#include <gtest/gtest.h>

typedef hpc :: FastCauldronEnvironmentConfigurationTokenizer Tok;

// case 1: empty string
TEST(FastCauldronEnvironmentConfigurationTokenizer, EmptyString)
{
   Tok tok("");
   EXPECT_TRUE( tok.hasMore());

   std::string t1, m1;
   tok.next(t1, m1);
   
   EXPECT_TRUE( t1.empty() );
   EXPECT_TRUE( m1.empty() );

   EXPECT_FALSE(tok.hasMore());
}

// case 2: one token
TEST(FastCauldronEnvironmentConfigurationTokenizer, OneToken)
{
   Tok tok("one token");
   EXPECT_TRUE( tok.hasMore());
   std::string t1, m1;
   tok.next( t1, m1);

   EXPECT_EQ( "one token", t1 );
   EXPECT_TRUE( m1.empty() );
   EXPECT_FALSE(tok.hasMore() );
}

// case 3: one marker
TEST(FastCauldronEnvironmentConfigurationTokenizer, OneMarker)
{
   Tok tok("{marker}");
   EXPECT_TRUE( tok.hasMore() );
   std::string t1, m1;
   tok.next( t1, m1);

   EXPECT_TRUE( t1.empty());
   EXPECT_EQ( "marker", m1 );

   EXPECT_TRUE( tok.hasMore() );

   std::string t2, m2;
   tok.next( t2, m2);
   EXPECT_TRUE( t2.empty());
   EXPECT_TRUE( m2.empty() );
   EXPECT_FALSE(tok.hasMore() );
}

// case 4: token + marker + token
TEST(FastCauldronEnvironmentConfigurationTokenizer, TokenMarkerToken)
{
   Tok tok("t1{m1}t2");
   EXPECT_TRUE( tok.hasMore() );
   std::string t1, m1;
   tok.next( t1, m1);

   EXPECT_EQ( "t1" , t1 );
   EXPECT_EQ( "m1", m1 );

   EXPECT_TRUE( tok.hasMore() );

   std::string t2, m2;
   tok.next( t2, m2);
   EXPECT_EQ( "t2", t2 );
   EXPECT_TRUE( m2.empty() );
   EXPECT_FALSE(tok.hasMore() );
}


// case 5: tokens + marker + marker + token
TEST(FastCauldronEnvironmentConfigurationTokenizer, TonerMarkerMarkerToken)
{
   Tok tok("t1{m1}{m2}t3");
   EXPECT_TRUE( tok.hasMore() );
   std::string t1, m1;
   tok.next( t1, m1);

   EXPECT_EQ( "t1" , t1 );
   EXPECT_EQ( "m1", m1 );

   EXPECT_TRUE( tok.hasMore() );

   std::string t2, m2;
   tok.next( t2, m2);
   EXPECT_TRUE( t2.empty());
   EXPECT_EQ( "m2" , m2 );
   EXPECT_TRUE( tok.hasMore() );

   std::string t3, m3;
   tok.next( t3, m3);
   EXPECT_EQ( "t3", t3 );
   EXPECT_TRUE( m3.empty() );
   EXPECT_FALSE( tok.hasMore() );
}


