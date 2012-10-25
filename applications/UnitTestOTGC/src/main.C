#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <memory>
using namespace std;

#include "TestingCode.h"
#include <math.h>



int main(int argc, char *argv[])
{
   if(argc==1)
   {
      std::cout<<"No test file...aborting"<<std::endl;
   } 
   const std::string testFileFullPathName = argv[1];
   try
   {
      UnitTestType theType = UnitTestFactory::determineTestType(testFileFullPathName);

      UnitTestFactory theUnitTestFactory = Singleton<UnitTestFactory>::instance();

      auto_ptr<UnitTest> theTest (theUnitTestFactory.createTest(theType,testFileFullPathName));

      theTest->execute();
   }
   catch(...){cout<<"Unknown exception caught..."<<endl;}


   return 0;

}


