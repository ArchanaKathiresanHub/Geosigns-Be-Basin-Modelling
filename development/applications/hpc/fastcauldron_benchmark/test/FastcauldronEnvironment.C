#include "fastcauldronenvironment.h"
#include "system.h"

#include <cassert>
#include <iostream>

void test_FastCauldronEnvironment_Configuration_Tokenizer()
{
   typedef hpc::FastCauldronEnvironment::Configuration::Tokenizer Tok;

   // case 1: empty string
   {
      Tok tok("");
      assert( tok.hasMore());

      std::string t1, m1;
      tok.next(t1, m1);
      
      assert( t1.empty() );
      assert( m1.empty() );

      assert( !tok.hasMore());
   }

   // case 2: one token
   {
      Tok tok("one token");
      assert( tok.hasMore());
      std::string t1, m1;
      tok.next( t1, m1);

      assert( t1 == "one token");
      assert( m1.empty() );
      assert( !tok.hasMore() );
   }

   // case 3: one marker
   {
      Tok tok("{marker}");
      assert( tok.hasMore() );
      std::string t1, m1;
      tok.next( t1, m1);

      assert( t1.empty());
      assert( m1 == "marker");

      assert( tok.hasMore() );

      std::string t2, m2;
      tok.next( t2, m2);
      assert( t2.empty());
      assert( m2.empty() );
      assert( !tok.hasMore() );
   }

   // case 4: token + marker + token
   {
      Tok tok("t1{m1}t2");
      assert( tok.hasMore() );
      std::string t1, m1;
      tok.next( t1, m1);

      assert( t1 == "t1" );
      assert( m1 == "m1");

      assert( tok.hasMore() );

      std::string t2, m2;
      tok.next( t2, m2);
      assert( t2 == "t2");
      assert( m2.empty() );
      assert( !tok.hasMore() );
   }


   // case 5: tokens + marker + marker + token
   {
      Tok tok("t1{m1}{m2}t3");
      assert( tok.hasMore() );
      std::string t1, m1;
      tok.next( t1, m1);

      assert( t1 == "t1" );
      assert( m1 == "m1");

      assert( tok.hasMore() );

      std::string t2, m2;
      tok.next( t2, m2);
      assert( t2.empty());
      assert( m2 == "m2" );
      assert( tok.hasMore() );

      std::string t3, m3;
      tok.next( t3, m3);
      assert( t3 == "t3");
      assert( m3.empty() );
      assert( ! tok.hasMore() );
   }
}

void test_FastCauldronEnvironment_Configuration_readTemplates()
{
   typedef std::pair<const hpc::FastCauldronEnvironment::VersionID, std::string > Pair;
   typedef hpc::FastCauldronEnvironment::Configuration Configuration;

   // case 1: Empty configuration file

   {  std::istringstream input("");

      Configuration c;
      c.readTemplates(input);

      assert( c.m_runTemplates.empty() );
   }

   // case 2: One empty environment
   {  std::istringstream input("[VERSION_1]");

      Configuration c;
      c.readTemplates(input);

      assert( c.m_runTemplates.size() == 1 );
      Pair pairs[] = { Pair("VERSION_1", "") };
      assert( std::equal( c.m_runTemplates.begin(), c.m_runTemplates.end(), pairs) );
   }

   // case 3: Two empty environments
   {  std::istringstream input("[VERSION_1]\n[VERSION_2]");

      Configuration c;
      c.readTemplates(input);

      assert( c.m_runTemplates.size() == 2 );
      Pair pairs[] = { Pair("VERSION_1", ""), Pair("VERSION_2", "") };
      assert( std::equal( c.m_runTemplates.begin(), c.m_runTemplates.end(), pairs) );
   }

   // case 4: One non-empty environment
   {  std::istringstream input("[VERSION_1]\nA\nB\nC\n");

      Configuration c;
      c.readTemplates(input);

      assert( c.m_runTemplates.size() == 1 );
      Pair pairs[] = { Pair("VERSION_1", "A\nB\nC\n") };
      assert( std::equal( c.m_runTemplates.begin(), c.m_runTemplates.end(), pairs) );
   }

   // case 5: One non-empty environment with some garbage 
   {  std::istringstream input("Som\ne garbage\n[VERSION_1]and even more\nA\nB\nC\n");

      Configuration c;
      c.readTemplates(input);

      assert( c.m_runTemplates.size() == 1 );
      Pair pairs[] = { Pair("VERSION_1", "A\nB\nC\n") };
      assert( std::equal( c.m_runTemplates.begin(), c.m_runTemplates.end(), pairs) );
   }

   // case 6: Three non-empty environments
   {  std::istringstream input("[VERSION_1]\nA\nB\nC\n[VERSION_2]\nD\nE\n[VERSION_3]\nF\n");

      Configuration c;
      c.readTemplates(input);

      assert( c.m_runTemplates.size() == 3 );
      Pair pairs[] = { Pair("VERSION_1", "A\nB\nC\n"), Pair("VERSION_2", "D\nE\n"), Pair("VERSION_3", "F\n") };
      assert( std::equal( c.m_runTemplates.begin(), c.m_runTemplates.end(), pairs) );
   }

}

void test_FastCauldronEnvironment_Configuration_getRunScript()
{
   typedef hpc::FastCauldronEnvironment::Configuration Configuration;
   std::istringstream input(
         "[VERSION_1]A\nB={PROCS}\n{FC_PARAMS} {INPUT}\n{OUTPUT}\n{MPI_PARAMS}\n{INPUT}\n"
         "[VERSION_3]A\nB={BLA}\n"
         );
   Configuration c;
   c.readTemplates(input);
  
   bool caught = false;

   // case 1: retrieving environment of non-existing version
   caught = false;
   try
   {
      std::vector<std::string> noParams;
      c.getRunScript( "VERSION_2", 3, noParams, "a", "b", noParams);
   }
   catch( hpc::FastCauldronEnvironment::Exception & e)
   {
      caught = true;
   }
   assert( caught );


   // case 2: retrieving environment that has unrecognized parameter
   caught = false;
   try
   {
      std::vector<std::string> noParams;
      c.getRunScript( "VERSION_3", 3, noParams, "a", "b", noParams);
   }
   catch( hpc::FastCauldronEnvironment::Exception & e)
   {
      caught = true;
   }
   assert( caught );

   // case 3: normal example
   {
      std::vector<std::string> mpiParams, fcParams;
      mpiParams.push_back("-one");
      mpiParams.push_back("-two");

      fcParams.push_back("--uno");
      fcParams.push_back("--duo");

      assert( "B=3\n--uno --duo a\nb\n-one -two\na\n"
            == c.getRunScript( "VERSION_1", 3, mpiParams, "a", "b", fcParams)
          );

   }
}

void test_FastCauldronEnvironment_jobHasRanBefore()
{
   typedef hpc::FastCauldronEnvironment FCE;
   struct Path : hpc::Path { Path() : hpc::Path("") {} };
   // case 1: parent dir does not exist => return false
   struct Case1Path : Path { FileType getFileType() const { return NotExists; } };
   assert( not FCE::jobHasRanBefore( Case1Path(), "benchmark_1") );

   // case 2: parent dir is not a directory but a file => failure
   struct Case2Path : Path { FileType getFileType() const { return Regular; } };
   bool caught = false;
   try { FCE::jobHasRanBefore( Case2Path(), "benchmark_1"); }
   catch(FCE::Exception & e) { caught = true; }
   assert( caught );

   // case 3: project dir does not exist => return false
   struct Case3Path : Path { 
      FileType getFileType() const { return Directory; } 
      
      struct Case3APath : Path {
         FileType getFileType() const { return NotExists; }
      };
      boost::shared_ptr< hpc::Path > getDirectoryEntry(const std::string & name) const
      {
         assert( name == "benchmark_1");
         return boost::shared_ptr< hpc::Path >( dynamic_cast<hpc::Path *>(new Case3APath) );
      }
   };
   assert( not FCE::jobHasRanBefore( Case3Path(), "benchmark_1"));


   // case 4: project dir is not a directory by a file => failure
   struct Case4Path : Path { 
      FileType getFileType() const { return Directory; } 
      
      struct Case4APath : Path {
         FileType getFileType() const { return Regular; }
      };
      boost::shared_ptr< hpc::Path > getDirectoryEntry(const std::string & name) const
      {
         assert( name == "benchmark_1");
         return boost::shared_ptr< hpc::Path >( dynamic_cast<hpc::Path *>(new Case4APath) );
      }
   };
   caught = false;
   try { FCE::jobHasRanBefore( Case2Path(), "benchmark_1"); }
   catch(FCE::Exception & e) { caught = true; }
   assert( caught );

   // case 5: project dir does exist and but does not contain a regular file 'output' => failure
   struct Case5Path : Path { 
      FileType getFileType() const { return Directory; } 
      
      struct Case5BPath : Path {
         FileType getFileType() const { return Directory; }
      };

      struct Case5APath : Path {
         FileType getFileType() const { return Directory; }

         boost::shared_ptr< hpc::Path > getDirectoryEntry(const std::string & name) const
         {
            assert( name == "output");
            return boost::shared_ptr< hpc::Path >( dynamic_cast<hpc::Path*>(new Case5BPath) );
         }
      };
      boost::shared_ptr< hpc::Path > getDirectoryEntry(const std::string & name) const
      {
         assert( name == "benchmark_1");
         return boost::shared_ptr< hpc::Path >( dynamic_cast<hpc::Path*>(new Case5APath) );
      }
   };
   caught = false;
   try { FCE::jobHasRanBefore( Case2Path(), "benchmark_1"); }
   catch(FCE::Exception & e) { caught = true; }
   assert( caught );

   // case 6: project dir does exist and contains a regular file 'output' => true
   struct Case6Path : Path { 
      FileType getFileType() const { return Directory; } 
      
      struct Case6BPath : Path {
         FileType getFileType() const { return Regular; }
      };

      struct Case6APath : Path {
         FileType getFileType() const { return Directory; }

         boost::shared_ptr< hpc::Path > getDirectoryEntry(const std::string & name) const
         {
            assert( name == "output");
            return boost::shared_ptr< hpc::Path >(dynamic_cast<hpc::Path*>( new Case6BPath ) );
         }
      };
      boost::shared_ptr< hpc::Path > getDirectoryEntry(const std::string & name) const
      {
         assert( name == "benchmark_1");
         return boost::shared_ptr< hpc::Path >( dynamic_cast<hpc::Path*>(new Case6APath) );
      }
   };
   assert(  FCE::jobHasRanBefore( Case6Path(), "benchmark_1"));
}

int main(int argc, char ** argv)
{
   if (argc < 2)
   {
      std::cerr << "Command line parameter is missing" << std::endl;
      return 1;
   }

   if (std::strcmp(argv[1], "FastCauldronEnvironment::Configuration::Tokenizer")==0)
      test_FastCauldronEnvironment_Configuration_Tokenizer();
   else if (std::strcmp(argv[1], "FastCauldronEnvironment::Configuration::readTemplates")==0)
      test_FastCauldronEnvironment_Configuration_readTemplates();
   else if (std::strcmp(argv[1], "FastCauldronEnvironment::Configuration::getRunScript")==0)
      test_FastCauldronEnvironment_Configuration_getRunScript();
   else if (std::strcmp(argv[1], "FastCauldronEnvironment::jobHasRanBefore")==0)
      test_FastCauldronEnvironment_jobHasRanBefore();
   else
   {
      std::cerr << "Unknown test" << std::endl;
      return 1;
   }

   return 0;
}
