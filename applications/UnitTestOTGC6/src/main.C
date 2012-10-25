#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <memory>
using namespace std;

#include "TestingCode.h"
#include <math.h>

void showHelp();

int main(int argc, char *argv[])
{
   if(argc == 1) {
      showHelp();
      exit(1);
   } 

   std::string testFileFullPathName;
   bool simulateGX5 = false;
   bool approxFlag = true;

   int argn;
   for(argn = 1; argn < argc; ++ argn) {
      if(strcmp(argv[argn], "-gen5") == 0){
         simulateGX5 = true;
      } else if(strcmp(argv[argn], "-na") == 0){
         approxFlag = false;
      } else if(strcmp(argv[argn], "-help") == 0 || strcmp(argv[argn], "-h") == 0){
          showHelp();
          exit(1);
      } else {
         testFileFullPathName = argv[argn];
      }
   }   
   if(testFileFullPathName.length() == 0) {
      std::cout << "No test file...aborting" << std::endl;
      showHelp();
      exit(1);
   }

   try {
      UnitTestType theType = UnitTestFactory::determineTestType(testFileFullPathName);

      UnitTestFactory theUnitTestFactory = Singleton<UnitTestFactory>::instance();
      
      auto_ptr<UnitTest> theTest (theUnitTestFactory.createTest(theType, testFileFullPathName, simulateGX5, approxFlag));

      theTest->execute();
   } catch(int &ex) {
      if(ex != FILE_NOT_FOUND) {
         cout << "Unknown exception caught..." << endl;
      }
   } catch(...) {
      cout << "Unknown exception caught..." << endl;
   }

   return 0;
}
void showHelp() 
{
   std::cerr << " Usage: " << endl;
   std::cerr << "     UnitTest [-na] [-gen5] <input file>" << endl;
   std::cerr << "              <input file> - input data file;" << endl; 
   std::cerr << "              [-na]        - don't do approximation in concentration;" << endl; 
   std::cerr << "              [-gen5]      - Genex5 simulaion." << endl; 
   std::cerr << " Example: UnitTest  -na -gen5 BenchmarkTests/Input/VBATestingSet.txt" << endl; 
   std::cerr << " Note: Output file and Config file (including paths) are specified inside input data file. Input data file is a file genereted by VBA-Genex code."<< endl; 
}
   
