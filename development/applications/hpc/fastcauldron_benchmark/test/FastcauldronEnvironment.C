#include "fastcauldronenvironment.h"
#include "system.h"

#include <iostream>
#include <gtest/gtest.h>

typedef hpc::FastCauldronEnvironment FCE;
typedef FCE::Configuration Configuration;
typedef std::pair<const FCE::VersionID, std::string > Pair;

struct Path : hpc::Path { Path() : hpc::Path("") {} };


// case 1: Empty configuration file
TEST(FastCauldronEnvironmentConfigurationReadTemplates, EmptyConfigurationFile)
{  std::istringstream input("");

   Configuration c(input);

   EXPECT_THROW( c.getRunTemplate("VERSION_1"), FCE::Exception );
}

// case 2: One empty environment
TEST(FastCauldronEnvironmentConfigurationReadTemplates, OneEmptyEnvironment)
{  std::istringstream input("[VERSION_1]");

   Configuration c(input);

   EXPECT_EQ( "", c.getRunTemplate("VERSION_1") );
}

// case 3: Two empty environments
TEST(FastCauldronEnvironmentConfigurationReadTemplates, TwoEmptyEnvironments )
{  std::istringstream input("[VERSION_1]\n[VERSION_2]");

   Configuration c(input);

   EXPECT_EQ( "", c.getRunTemplate("VERSION_1") );
   EXPECT_EQ( "", c.getRunTemplate("VERSION_2") );
}

// case 4: One non-empty environment
TEST(FastCauldronEnvironmentConfigurationReadTemplates, OneNonEmptyEnvironment )
{  std::istringstream input("[VERSION_1]\nA\nB\nC\n");

   Configuration c(input);

   EXPECT_EQ( "A\nB\nC\n", c.getRunTemplate("VERSION_1") );
   EXPECT_THROW( c.getRunTemplate("VERSION_2"), FCE::Exception);
}

// case 5: One non-empty environment with some garbage 
TEST(FastCauldronEnvironmentConfigurationReadTemplates, OneNonEmptyEnvironmentPlusGarbage )
{  std::istringstream input("Som\ne garbage\n[VERSION_1]and even more\nA\nB\nC\n");

   Configuration c(input);

   EXPECT_EQ( "A\nB\nC\n", c.getRunTemplate("VERSION_1") );
   EXPECT_THROW( c.getRunTemplate("VERSION_2"), FCE::Exception);
}

// case 6: Three non-empty environments
TEST(FastCauldronEnvironmentConfigurationReadTemplates, ThreeNonEmptyEnvironments )
{  std::istringstream input("[VERSION_1]\nA\nB\nC\n[VERSION_2]\nD\nE\n[VERSION_3]\nF\n");

   Configuration c(input);

   EXPECT_EQ( "A\nB\nC\n", c.getRunTemplate("VERSION_1") );
   EXPECT_EQ( "D\nE\n", c.getRunTemplate("VERSION_2") );
   EXPECT_EQ( "F\n", c.getRunTemplate("VERSION_3") );
}


class FastCauldronEnvironmentConfigurationTest : public ::testing::Test
{
protected:
   FastCauldronEnvironmentConfigurationTest()
      : m_config(input())
   {
   }


   static Configuration input()
   {
      std::istringstream stream(
            "[VERSION_1]A\nB={PROCS}\n{FC_PARAMS} {INPUT}\n{OUTPUT}\n{MPI_PARAMS}\n{INPUT}\n"
            "[VERSION_3]A\nB={BLA}\n"
            );
      return Configuration(stream);
   }

   Configuration m_config;
   const std::vector< std::string > m_noParams; 
};

// case 1: retrieving environment of non-existing version
TEST_F(FastCauldronEnvironmentConfigurationTest,GetRunScriptOfNonExistingVersion)
{
   EXPECT_THROW( m_config.getRunScript( "VERSION_2", 3, m_noParams, "a", "b", m_noParams),
      FCE::Exception 
   );
}

// case 2: retrieving environment that has unrecognized parameter
TEST_F(FastCauldronEnvironmentConfigurationTest,GetRunScriptHavingUnrecognizedParameter)
{
   EXPECT_THROW( m_config.getRunScript( "VERSION_3", 3, m_noParams, "a", "b", m_noParams),
      FCE::Exception
   );
}

// case 3: normal example
TEST_F(FastCauldronEnvironmentConfigurationTest,GetRunScriptNormalRun)
{
   std::vector<std::string> mpiParams, fcParams;
   mpiParams.push_back("-one");
   mpiParams.push_back("-two");

   fcParams.push_back("--uno");
   fcParams.push_back("--duo");

   EXPECT_EQ( "B=3\n--uno --duo a\nb\n-one -two\na\n",
         m_config.getRunScript( "VERSION_1", 3, mpiParams, "a", "b", fcParams)
       );

}


// case 1: parent dir does not exist => return false
TEST(FastCauldronEnvironmentJobHasRanBefore, parentDirDoesNotExist)
{
   struct Case1Path : Path { FileType getFileType() const { return NotExists; } };
   EXPECT_FALSE( FCE::jobHasRanBefore( Case1Path(), "benchmark_1") );
}

// case 2: parent dir is not a directory but a file => failure
TEST(FastCauldronEnvironmentJobHasRanBefore, parentDirIsNotADir)
{
   struct Case2Path : Path { FileType getFileType() const { return Regular; } };
   EXPECT_THROW( FCE::jobHasRanBefore( Case2Path(), "benchmark_1"), FCE::Exception );
}

// case 3: project dir does not exist => return false
TEST(FastCauldronEnvironmentJobHasRanBefore, projectDirDoesNotExist)
{
   struct Case3Path : Path { 
      FileType getFileType() const { return Directory; } 
      
      struct Case3APath : Path {
         FileType getFileType() const { return NotExists; }
      };
      boost::shared_ptr< hpc::Path > getDirectoryEntry(const std::string & name) const
      {
         EXPECT_EQ( "benchmark_1", name );
         return boost::shared_ptr< hpc::Path >( dynamic_cast<hpc::Path *>(new Case3APath) );
      }
   };
   EXPECT_FALSE( FCE::jobHasRanBefore( Case3Path(), "benchmark_1"));
}


// case 4: project dir is not a directory by a file => failure
TEST(FastCauldronEnvironmentJobHasRanBefore, projectDirIsNotADir)
{
   struct Case4Path : Path { 
      FileType getFileType() const { return Directory; } 
      
      struct Case4APath : Path {
         FileType getFileType() const { return Regular; }
      };
      boost::shared_ptr< hpc::Path > getDirectoryEntry(const std::string & name) const
      {
         EXPECT_EQ( "benchmark_1", name );
         return boost::shared_ptr< hpc::Path >( dynamic_cast<hpc::Path *>(new Case4APath) );
      }
   };
   EXPECT_THROW( FCE::jobHasRanBefore( Case4Path(), "benchmark_1"), FCE::Exception);
}

// case 5: project dir does exist and but does not contain a regular file 'output' => failure
TEST(FastCauldronEnvironmentJobHasRanBefore, projectDirContainsIrregularFileOutput)
{
   struct Case5Path : Path { 
      FileType getFileType() const { return Directory; } 
      
      struct Case5BPath : Path {
         FileType getFileType() const { return Directory; }
      };

      struct Case5APath : Path {
         FileType getFileType() const { return Directory; }

         boost::shared_ptr< hpc::Path > getDirectoryEntry(const std::string & name) const
         {
            EXPECT_EQ( "output", name );
            return boost::shared_ptr< hpc::Path >( dynamic_cast<hpc::Path*>(new Case5BPath) );
         }
      };
      boost::shared_ptr< hpc::Path > getDirectoryEntry(const std::string & name) const
      {
         EXPECT_EQ( "benchmark_1", name );
         return boost::shared_ptr< hpc::Path >( dynamic_cast<hpc::Path*>(new Case5APath) );
      }
   };
   EXPECT_THROW( FCE::jobHasRanBefore( Case5Path(), "benchmark_1"), FCE::Exception);
}

// case 6: project dir does exist and contains a regular file 'output' => true
TEST(FastCauldronEnvironmentJobHasRanBefore, projectDirContainsRegularFileOutput)
{
   struct Case6Path : Path { 
      FileType getFileType() const { return Directory; } 
      
      struct Case6BPath : Path {
         FileType getFileType() const { return Regular; }
      };

      struct Case6APath : Path {
         FileType getFileType() const { return Directory; }

         boost::shared_ptr< hpc::Path > getDirectoryEntry(const std::string & name) const
         {
            EXPECT_EQ( "output", name );
            return boost::shared_ptr< hpc::Path >(dynamic_cast<hpc::Path*>( new Case6BPath ) );
         }
      };
      boost::shared_ptr< hpc::Path > getDirectoryEntry(const std::string & name) const
      {
         EXPECT_EQ( "benchmark_1", name );
         return boost::shared_ptr< hpc::Path >( dynamic_cast<hpc::Path*>(new Case6APath) );
      }
   };
   EXPECT_TRUE( FCE::jobHasRanBefore( Case6Path(), "benchmark_1"));
}

