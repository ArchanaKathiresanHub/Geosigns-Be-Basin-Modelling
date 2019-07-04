#include "../src/parseUtilities.h"

#include <gtest/gtest.h>

#include <string>
#include <cstring>

TEST( Track1dParseUtilities, SplitStringComma )
{
  char* firstPart;
  char* secondPart;
  bool success;

  char str1[21] = "firstPart,secondPart";
  success = DataExtraction::ParseUtilities::splitString( str1, ',', firstPart, secondPart );
  EXPECT_TRUE( success );
  EXPECT_EQ( std::string(firstPart), "firstPart" );
  EXPECT_EQ( std::string(secondPart), "secondPart" );
}

TEST( Track1dParseUtilities, SplitStringColon )
{
  char* firstPart;
  char* secondPart;
  bool success;

  char str1[23] = "try:differentseparator";
  success = DataExtraction::ParseUtilities::splitString( str1, ':', firstPart, secondPart );
  EXPECT_TRUE( success );
  EXPECT_EQ( std::string(firstPart), "try" );
  EXPECT_EQ( std::string(secondPart), "differentseparator" );
}

TEST( Track1dParseUtilities, ParseCoordinates )
{
  char coordinatesString[16] = "1.2,3.4,5.6,7.8";
  DataExtraction::DoublePairVector vec = DataExtraction::ParseUtilities::parseCoordinates( coordinatesString );
  EXPECT_EQ( vec.size(), 2 );
  EXPECT_DOUBLE_EQ( vec[0].first, 1.2);
  EXPECT_DOUBLE_EQ( vec[0].second, 3.4);
  EXPECT_DOUBLE_EQ( vec[1].first, 5.6);
  EXPECT_DOUBLE_EQ( vec[1].second, 7.8);
}

TEST( Track1dParseUtilities, ParseAges )
{
  char agesString[13] = "0,10-100,200";
  DataExtraction::DoubleVector vec = DataExtraction::ParseUtilities::parseAges( agesString );
  EXPECT_EQ( vec.size(), 8 );
  EXPECT_DOUBLE_EQ( vec[0], 0);
  EXPECT_DOUBLE_EQ( vec[1], -1);
  EXPECT_DOUBLE_EQ( vec[2], 10);
  EXPECT_DOUBLE_EQ( vec[3], 100);
  EXPECT_DOUBLE_EQ( vec[4], -1);
  EXPECT_DOUBLE_EQ( vec[5], 200);
  EXPECT_DOUBLE_EQ( vec[6], -1);
  EXPECT_DOUBLE_EQ( vec[7], -1);
}

TEST( Track1dParseUtilities, ParseStrings )
{
  char stringsString[16] = "foo,bar,baz,bla";
  DataExtraction::StringVector vec = DataExtraction::ParseUtilities::parseStrings( stringsString );
  EXPECT_EQ( vec.size(), 4 );
  EXPECT_EQ( vec[0], "foo" );
  EXPECT_EQ( vec[1], "bar" );
  EXPECT_EQ( vec[2], "baz" );
  EXPECT_EQ( vec[3], "bla" );
}
