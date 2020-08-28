//                                                                      
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by CGI India Pvt. Ltd.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//


#include"gtest/gtest.h"
#include "../src/CrustalThicknessCalculator.h"
#include "../src/CrustalThicknessCalculatorFactory.h"
// petsc library
#include <petsc.h>
namespace fastctctest //  ALL in here is local to this translation unit
{
  
};

class TestCrustalThicknessCalculator : public ::testing::Test {
public:
    void SetUp()
    {
        int rank = 99999;
        MPI_Comm_rank(PETSC_COMM_WORLD, &rank);
        CrustalThicknessCalculatorFactory factory;
        crustalThicknessCalculator.reset(CrustalThicknessCalculator::createFrom(filename, &factory));
        ASSERT_NE(crustalThicknessCalculator, nullptr );
    }
    void TearDown() {

        //crustalThicknessCalculator->finalise(true);
        
        crustalThicknessCalculator.reset();
        delete[] argv;
        PetscFinalize();
    }
private:
    const static std::string filename; 
protected: 
    int argc; char** argv;
    std::unique_ptr<CrustalThicknessCalculator> crustalThicknessCalculator;
};

const std::string TestCrustalThicknessCalculator::filename= "Project.project3d";

class TestCrustalThicknessCalculatorMerging : public TestCrustalThicknessCalculator {
    void SetUp() final
    {
        //argv[0], is the program's name.
        std::vector<std::string> argvList = {"fastctc.exe","-project","Project.project3d","-save","Project.project3d.ctc","-merge"};
        //while passing three arguments, argc is 4
        argc = argvList.size();
        argv = new char* [argvList.size()];
        for (int i = 0; i != argvList.size(); i++) {
            auto c = const_cast<char*>(argvList[i].c_str());
            argv[i] = c ;
        }
        auto ierr=PetscInitialize(&argc, &argv, (char*)0, PETSC_IGNORE); if (ierr) throw (std::runtime_error("PetscInitialize Failed")) ;
        TestCrustalThicknessCalculator::SetUp();
    }

};


TEST_F(TestCrustalThicknessCalculatorMerging, mockTest) {
    
    ASSERT_EQ( crustalThicknessCalculator->parseCommandLine(), true);

    ASSERT_EQ( crustalThicknessCalculator->deleteCTCPropertyValues(), false);

    // Have to see how to test these??
    //crustalThicknessCalculator->initialiseCTC();

    //crustalThicknessCalculator->run();
}



