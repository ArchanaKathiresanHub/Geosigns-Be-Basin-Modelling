#include "fastcauldronenvironment.h"
#include "system.h"

#include <iostream>
#include <gtest/gtest.h>

typedef hpc::FastCauldronEnvironment FCE;
typedef FCE::Configuration Configuration;
typedef Configuration::Tokenizer Tok;
typedef std::pair<const FCE::VersionID, std::string > Pair;

struct Path : hpc::Path { Path() : hpc::Path("") {} };


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


// case 1: Empty configuration file
TEST(FastCauldronEnvironmentConfigurationReadTemplates, EmptyConfigurationFile)
{  std::istringstream input("");

   Configuration c;
   c.readTemplates(input);

   ASSERT_TRUE( c.m_runTemplates.empty() );
}

// case 2: One empty environment
TEST(FastCauldronEnvironmentConfigurationReadTemplates, OneEmptyEnvironment)
{  std::istringstream input("[VERSION_1]");

   Configuration c;
   c.readTemplates(input);

   ASSERT_EQ( 1 , c.m_runTemplates.size() );
   Pair pairs[] = { Pair("VERSION_1", "") };
   ASSERT_TRUE( std::equal( c.m_runTemplates.begin(), c.m_runTemplates.end(), pairs) );
}

// case 3: Two empty environments
TEST(FastCauldronEnvironmentConfigurationReadTemplates, TwoEmptyEnvironments )
{  std::istringstream input("[VERSION_1]\n[VERSION_2]");

   Configuration c;
   c.readTemplates(input);

   ASSERT_EQ( 2 , c.m_runTemplates.size() );
   Pair pairs[] = { Pair("VERSION_1", ""), Pair("VERSION_2", "") };
   ASSERT_TRUE( std::equal( c.m_runTemplates.begin(), c.m_runTemplates.end(), pairs) );
}

// case 4: One non-empty environment
TEST(FastCauldronEnvironmentConfigurationReadTemplates, OneNonEmptyEnvironment )
{  std::istringstream input("[VERSION_1]\nA\nB\nC\n");

   Configuration c;
   c.readTemplates(input);

   ASSERT_EQ( 1 , c.m_runTemplates.size() );
   Pair pairs[] = { Pair("VERSION_1", "A\nB\nC\n") };
   ASSERT_TRUE( std::equal( c.m_runTemplates.begin(), c.m_runTemplates.end(), pairs) );
}

// case 5: One non-empty environment with some garbage 
TEST(FastCauldronEnvironmentConfigurationReadTemplates, OneNonEmptyEnvironmentPlusGarbage )
{  std::istringstream input("Som\ne garbage\n[VERSION_1]and even more\nA\nB\nC\n");

   Configuration c;
   c.readTemplates(input);

   ASSERT_EQ( 1 , c.m_runTemplates.size() );
   Pair pairs[] = { Pair("VERSION_1", "A\nB\nC\n") };
   ASSERT_TRUE( std::equal( c.m_runTemplates.begin(), c.m_runTemplates.end(), pairs) );
}

// case 6: Three non-empty environments
TEST(FastCauldronEnvironmentConfigurationReadTemplates, ThreeNonEmptyEnvironments )
{  std::istringstream input("[VERSION_1]\nA\nB\nC\n[VERSION_2]\nD\nE\n[VERSION_3]\nF\n");

   Configuration c;
   c.readTemplates(input);

   ASSERT_EQ( 3 , c.m_runTemplates.size() );
   Pair pairs[] = { Pair("VERSION_1", "A\nB\nC\n"), Pair("VERSION_2", "D\nE\n"), Pair("VERSION_3", "F\n") };
   ASSERT_TRUE( std::equal( c.m_runTemplates.begin(), c.m_runTemplates.end(), pairs) );
}


class FastCauldronEnvironmentConfigurationTest : public ::testing::Test
{
public:
   FastCauldronEnvironmentConfigurationTest()
      : m_config()
   {
      std::istringstream input(
            "[VERSION_1]A\nB={PROCS}\n{FC_PARAMS} {INPUT}\n{OUTPUT}\n{MPI_PARAMS}\n{INPUT}\n"
            "[VERSION_3]A\nB={BLA}\n"
            );
      m_config.readTemplates(input);
   }


private:
   Configuration m_config;
   const std::vector< std::string > m_noParams; 
};

// case 1: retrieving environment of non-existing version
TEST_F(FastCauldronEnvironmentConfigurationTest,GetRunScriptOfNonExistingVersion)
{
   EXPECT_THROW( m_config.getRunScript( "VERSION_2", 3, m_noParams, "a", "b", m_noParams),
      hpc::FastCauldronEnvironment::Exception 
   );
}

// case 2: retrieving environment that has unrecognized parameter
TEST_F(FastCauldronEnvironmentConfigurationTest,GetRunScriptHavingUnrecognizedParameter)
{
   EXPECT_THROW( m_config.getRunScript( "VERSION_3", 3, m_noParams, "a", "b", m_noParams),
      hpc::FastCauldronEnvironment::Exception
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

