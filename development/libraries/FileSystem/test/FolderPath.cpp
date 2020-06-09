//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <fstream>
#include <string>
#include "FilePath.h"
#include "FolderPath.h"
#include <boost/filesystem/operations.hpp>
#include <gtest/gtest.h>

static const std::string s_folder = "FolderPath_unittest_folder";

namespace supportFunc
{
   void createFile(const std::string & f)
   {
      std::ofstream outfile(f);
      outfile.close();
   }
}

TEST( FolderPath, EmptyConstructor )
{
   ibs::FolderPath p(std::string(""));
   EXPECT_EQ( p.path(), std::string(".") );
   EXPECT_TRUE( p.exists() );
   EXPECT_EQ( p.cpath(), std::string(".") );
   EXPECT_EQ( p.cpath()[0], '.' );
   EXPECT_EQ( p.size(), 1 );
   EXPECT_EQ( p[0], std::string(".") );
   EXPECT_EQ( p[100], std::string() );
   EXPECT_EQ( p.cutLast().path(), std::string("") );
}

TEST( FolderPath, InputConstructor )
{
   ibs::FolderPath p(s_folder);
   EXPECT_EQ( p.path(), s_folder );
   EXPECT_FALSE( p.exists() );
   EXPECT_EQ( p.cpath(), s_folder );
   EXPECT_EQ( p.size(), 1 );
   const char * c = s_folder.c_str();
   for(size_t i = 0; i < s_folder.size(); ++i )
   {
      EXPECT_EQ( p.cpath()[i], c[i] );
   }
   EXPECT_EQ( p[0], s_folder );
   EXPECT_EQ( p[100], std::string() );
   EXPECT_EQ( p.cutLast().path(), std::string("") );
}

TEST( FolderPath, createAndRemove )
{
   ibs::FolderPath p(s_folder);
   try
   {
      p.create();
      EXPECT_TRUE( p.exists() );
      EXPECT_TRUE( p.empty() );
      p.remove();
      EXPECT_FALSE( p.exists() );
   }
   catch(...)
   {
      if(p.exists()) p.remove();
      FAIL();
   }
}

TEST( FolderPath, clean )
{
   ibs::FolderPath p(s_folder);
   try
   {
      p.create();
      EXPECT_TRUE( p.exists() );
      EXPECT_TRUE( p.empty() );
      ibs::FilePath f(p.fullPath());
      f << "TEST";
      supportFunc::createFile( f.fullPath().path() );
      EXPECT_TRUE( f.exists() );
      EXPECT_FALSE( p.empty() );
      p.clean();
      EXPECT_FALSE( f.exists() );
      EXPECT_TRUE( p.empty() );
      p.remove();
      EXPECT_FALSE( p.exists() );
   }
   catch(...)
   {
      if(p.exists()) p.remove();
      FAIL();
   }
}
