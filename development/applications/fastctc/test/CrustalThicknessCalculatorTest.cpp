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
        std::vector<std::string> argvList = {"fastctc.exe","-project","Project.project3d","-save","Project_out.project3d","-merge"};
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

    CrustalThicknessCalculatorFactory factory;
    std::unique_ptr<CrustalThicknessCalculator> crustalThicknessCalculator(CrustalThicknessCalculator::createFrom("Project.project3d", &factory));  
    if (crustalThicknessCalculator == nullptr) FAIL();

    ASSERT_EQ( crustalThicknessCalculator->parseCommandLine(), true);
    ASSERT_EQ( crustalThicknessCalculator->deleteCTCPropertyValues(), false);
    // Have to see how to test these??
    crustalThicknessCalculator->initialiseCTC();
    crustalThicknessCalculator->run();
    crustalThicknessCalculator->finalise(true);
    crustalThicknessCalculator.reset();

    DataAccess::Interface::ObjectFactory factory1;
    DataAccess::Interface::ObjectFactory* factoryptr = &factory1;
    std::unique_ptr<DataAccess::Interface::ProjectHandle> ph;
    
    try
    {
        ph.reset(DataAccess::Interface::OpenCauldronProject("Project_out.project3d", factoryptr));//Will use the output p3d file created by CTC for validating the crustal thickness validation below
    }
    catch (...)
    {
        FAIL() << "Unexpected exception caught";
    }
    //// Check continental crustal history
    database::Table* contCrustalHistoryIoTbl = ph->getTable("ContCrustalThicknessIoTbl"); 
    if (contCrustalHistoryIoTbl == nullptr) FAIL();
    EXPECT_EQ(contCrustalHistoryIoTbl->size(), 2);
    database::Record* rec1 = contCrustalHistoryIoTbl->getRecord(0);
    double age1= rec1->getValue<double>("Age");
    double thickness1 = rec1->getValue<double>("Thickness");
    std::string thicknessGrid1 = rec1->getValue<std::string>("ThicknessGrid");
    EXPECT_EQ(age1, 0);
    database::Record* rec2 = contCrustalHistoryIoTbl->getRecord(1);
    double age2 = rec2->getValue<double>("Age");
    double thickness2 = rec2->getValue<double>("Thickness");
    std::string thicknessGrid2 = rec2->getValue<std::string>("ThicknessGrid");
    EXPECT_EQ(age2, 155);
    EXPECT_EQ(thickness1, thickness2);
    EXPECT_EQ(thicknessGrid1, thicknessGrid2);//Depending upon the RiftingHistory provided for the scenario used for testing

    //// Check oceanic crustal history
    database::Table* oceaCrustalHistoryIoTbl = ph->getTable("OceaCrustalThicknessIoTbl");
    if (oceaCrustalHistoryIoTbl == nullptr) FAIL();
    EXPECT_EQ(oceaCrustalHistoryIoTbl->size(), 2);
    EXPECT_EQ(oceaCrustalHistoryIoTbl->size(), contCrustalHistoryIoTbl->size());
    rec1 = oceaCrustalHistoryIoTbl->getRecord(0);
    age1 = rec1->getValue<double>("Age");
    thickness1 = rec1->getValue<double>("Thickness");
    thicknessGrid1 = rec1->getValue<std::string>("ThicknessGrid");
    EXPECT_EQ(age1, 0);
    rec2 = oceaCrustalHistoryIoTbl->getRecord(1);
    age2 = rec2->getValue<double>("Age");
    thickness2 = rec2->getValue<double>("Thickness");
    thicknessGrid2 = rec2->getValue<std::string>("ThicknessGrid");
    EXPECT_EQ(age2, 155);
    EXPECT_EQ(thickness1, thickness2);
    EXPECT_EQ(thicknessGrid1, thicknessGrid2);//Depending upon the RiftingHistory provided for the scenario used for testing
            
}



