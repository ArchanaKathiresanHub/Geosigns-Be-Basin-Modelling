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
#include <boost/filesystem/operations.hpp>
#include <gtest/gtest.h>

static const std::string s_file = "FilePath.test";
static const std::string s_folder = "FilePath_unittest_folder";
static const std::string s_fileContent = "This is a test file for FilePath class.";

namespace supportFunc
{
   void createFile(const std::string & f)
   {
      std::ofstream outfile(f);
      outfile << s_fileContent;
      outfile.close();
   }
   bool createFolder(const std::string & f)
   {
      boost::filesystem::path dir(f);
      return boost::filesystem::create_directory(dir);
   }
   bool checkFileContent(const std::string & f)
   {
      std::ifstream in(f.c_str());
      std::string line;
      getline(in,line);
      in.close();
      return line == s_fileContent;
   }
}

TEST( FilePath, EmptyConstructor )
{
   ibs::FilePath p(std::string(""));
   EXPECT_EQ( p.path(), std::string(".") );
   EXPECT_TRUE( p.exists() );
   EXPECT_EQ( p.cpath(), std::string(".") );
   EXPECT_EQ( p.cpath()[0], '.' );
   EXPECT_EQ( p.size(), 1 );
   EXPECT_EQ( p[0], std::string(".") );
   EXPECT_EQ( p[100], std::string() );
   EXPECT_EQ( p.cutLast().path(), std::string("") );
}

TEST( FilePath, InputConstructor )
{
   ibs::FilePath p(s_file);
   EXPECT_EQ( p.path(), s_file );
   EXPECT_FALSE( p.exists() );
   EXPECT_EQ( p.cpath(), s_file );
   EXPECT_EQ( p.size(), 1 );
   const char * c = s_file.c_str();
   for(size_t i = 0; i < s_file.size(); ++i )
   {
      EXPECT_EQ( p.cpath()[i], c[i] );
   }
   EXPECT_EQ( p[0], s_file );
   EXPECT_EQ( p[100], std::string() );
   EXPECT_EQ( p.cutLast().path(), std::string("") );
}

TEST( FilePath, fileName )
{
   ibs::FilePath p(s_file);
   p = p.fullPath();
   EXPECT_EQ( p.fileName(), s_file );
}

TEST( FilePath, filePath )
{
   if( ibs::Path(s_folder).exists() ) boost::filesystem::remove_all( boost::filesystem::path(s_folder) );
   const bool rc = supportFunc::createFolder(s_folder);
   if(!rc) FAIL();
   const std::string fName = (ibs::Path(s_folder)<<s_file).fullPath().path();
   supportFunc::createFile( fName );
   ibs::FilePath p(fName);
   if(!p.exists())
   {
      boost::filesystem::remove_all( boost::filesystem::path(s_folder) );
      FAIL();
   }
   EXPECT_EQ( p.filePath(), ibs::Path(s_folder).fullPath().path() );
   boost::filesystem::remove_all( boost::filesystem::path(s_folder) );
}

TEST( FilePath, fileNameNoExtension1 )
{
   ibs::FilePath p(s_file);
   p = p.fullPath();
   EXPECT_EQ( p.fileNameNoExtension(), "FilePath" );
}

TEST( FilePath, fileNameNoExtension2 )
{
   ibs::FilePath p(s_file + std::string(".ext2"));
   p = p.fullPath();
   EXPECT_EQ( p.fileNameNoExtension(), "FilePath.test" );
}

TEST( FilePath, fileNameExtension1 )
{
   ibs::FilePath p(s_file);
   p = p.fullPath();
   EXPECT_EQ( p.fileNameExtension(), "test" );
}

TEST( FilePath, fileNameExtension2 )
{
   ibs::FilePath p(s_file + std::string(".ext2"));
   p = p.fullPath();
   EXPECT_EQ( p.fileNameExtension(), "ext2" );
}

TEST( FilePath, copyFile )
{
   ibs::FilePath p(s_file);
   p = p.fullPath().path();
   supportFunc::createFile( p.fullPath().path() );
   if(!p.exists()) FAIL();
   ibs::FilePath pCopy(p.fullPath().path()+std::string("_COPY"));
   EXPECT_TRUE( p.copyFile(pCopy) );
   if(!pCopy.exists())
   {
      p.remove();
      FAIL();
   }
   EXPECT_TRUE( supportFunc::checkFileContent(pCopy.path()) );
   p.remove();
   pCopy.remove();
}

TEST( FilePath, linkFile )
{
   ibs::FilePath p(s_file);
   if( p.exists() ) p.remove();
   p = p.fullPath().path();
   supportFunc::createFile( p.fullPath().path() );
   if(!p.exists()) FAIL();
   ibs::FilePath pLink(p.fullPath().path()+std::string("_LINK"));
   if( pLink.exists() ) pLink.remove();
#ifdef _MSC_VER
   // linkFile does not work on Windows
   EXPECT_FALSE( p.linkFile(pLink) );
   p.remove();
#else
   EXPECT_TRUE( p.linkFile(pLink) );
   if(!pLink.exists())
   {
      p.remove();
      FAIL();
   }
   EXPECT_TRUE( supportFunc::checkFileContent(pLink.path()) );
   p.remove();
   pLink.remove();
#endif
}
