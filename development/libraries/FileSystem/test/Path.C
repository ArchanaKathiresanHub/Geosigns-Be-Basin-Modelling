//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <string>
#include "Path.h"
#include <boost/filesystem/operations.hpp>
#include <gtest/gtest.h>

static const std::string s_file = "Path/File.test";

TEST( Path, applicationFullPath )
{
   const ibs::Path p( ibs::Path::applicationFullPath() );
   EXPECT_TRUE( p.exists() );
   EXPECT_TRUE( boost::filesystem::is_directory(p.path()) );
   // This is workaround because VS adds an extra folder (Debug/Release)
   EXPECT_TRUE( p.path().find( ibs::Path(std::string("")).fullPath().cutLast().path()) != std::string::npos );
}

TEST( Path, EmptyConstructor )
{
   ibs::Path p(std::string(""));
   EXPECT_EQ( p.path(), std::string(".") );
   EXPECT_TRUE( p.exists() );
   EXPECT_EQ( p.cpath(), std::string(".") );
   EXPECT_EQ( p.cpath()[0], '.' );
   EXPECT_EQ( p.size(), 1 );
   EXPECT_EQ( p[0], std::string(".") );
   EXPECT_EQ( p[100], std::string() );
   EXPECT_EQ( p.cutLast().path(), std::string("") );
}

TEST( Path, InputConstructor )
{
   ibs::Path p(s_file);
   EXPECT_EQ( p.path(), s_file );
   EXPECT_FALSE( p.exists() );
   EXPECT_EQ( p.cpath(), s_file );
   EXPECT_EQ( p.size(), 2 );
   const char * c = s_file.c_str();
   for(size_t i = 0; i < s_file.size(); ++i )
   {
      EXPECT_EQ( p.cpath()[i], c[i] );
   }
   EXPECT_EQ( p[0], std::string("Path") );
   EXPECT_EQ( p[1], std::string("File.test") );
   EXPECT_EQ( p[100], std::string() );
   EXPECT_EQ( p.cutLast().path(), std::string("Path") );
}

TEST( Path, OperatorAngledBrackets )
{
   ibs::Path p(std::string("TEST_FOLDER"));
   EXPECT_EQ( p.size(), 1 );
   p << std::string("TEST_FILE");
   EXPECT_EQ( p.size(), 2 );
#ifdef _MSC_VER
   EXPECT_EQ( p.path(), std::string("TEST_FOLDER\\TEST_FILE") );
#else
   EXPECT_EQ( p.path(), std::string("TEST_FOLDER/TEST_FILE") );
#endif
}
