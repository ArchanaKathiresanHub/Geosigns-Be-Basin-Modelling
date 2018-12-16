#include "../src/cmbAPI.h"

#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fstream>

#include "FilePath.h"

//#include <cmath>

#include <gtest/gtest.h>

static const double eps = 1.e-5;
static const double eps1 = 1.e-4;

class mbapiModelTest : public ::testing::Test
{
public:
   mbapiModelTest() { ; }
   ~mbapiModelTest() { ; }

   bool compareFiles(const char * projFile1, const char * projFile2);
   bool compareModels(mbapi::Model & model1, mbapi::Model & model2);

   static const char * m_sourceRockTestProject;
   static const char * m_testProject;
   static const char * m_lithologyTestProject;
   static const char * m_fluidTestProject;
   static const char * m_dupLithologyTestProject;
   static const char * m_mapsTestProject;
   static const char * m_nnTestProject;
   static const char * m_reservoirTestProject;
   static const char * m_CtcTestProject;
};

const char * mbapiModelTest::m_sourceRockTestProject = "SourceRockTesting.project3d";
const char * mbapiModelTest::m_lithologyTestProject = "LithologyTesting.project3d";
const char * mbapiModelTest::m_fluidTestProject = "FluidTesting.project3d";
const char * mbapiModelTest::m_testProject = "Project.project3d";
const char * mbapiModelTest::m_mapsTestProject = "MapsTesting.project3d";
const char * mbapiModelTest::m_dupLithologyTestProject = "DupLithologyTesting.project3d";
const char * mbapiModelTest::m_nnTestProject = "NNTesting.project3d";
const char * mbapiModelTest::m_reservoirTestProject = "ReservoirTesting.project3d";
const char * mbapiModelTest::m_CtcTestProject = "CtcTesting.project3d";


bool mbapiModelTest::compareFiles(const char * projFile1, const char * projFile2)
{
   struct stat filestatus;
   stat(projFile1, &filestatus);
   off_t case1Size = filestatus.st_size;
   stat(projFile2, &filestatus);

   bool ret = case1Size == filestatus.st_size;

   if (ret)
   {
      std::ifstream p1f(projFile1);
      std::ifstream p2f(projFile2);

      size_t lineNum = 0;
      while (ret && p1f.good() && p2f.good())
      {
         char buf1[2048];
         char buf2[2048];

         p1f.getline(buf1, sizeof(buf1) / sizeof(char));
         p2f.getline(buf2, sizeof(buf2) / sizeof(char));

         if (lineNum > 1 && strcmp(buf1, buf2)) // ignore 2 first lines because of filename and dates there
         {
            ret = false;
         }
         lineNum++;
      }
      // check that both files with the same state
      ret = ret && (p1f.good() == p2f.good());
   }
   return ret;
}

bool mbapiModelTest::compareModels(mbapi::Model & model1, mbapi::Model & model2)
{
   const char * projFile1 = "project_model_1.proj3d";
   const char * projFile2 = "project_model_2.proj3d";

   model1.saveModelToProjectFile(projFile1);
   model2.saveModelToProjectFile(projFile2);

   bool isTheSame = compareFiles(projFile1, projFile2);

   // clean files copy
   remove(projFile1);
   remove(projFile2);

   return isTheSame;
}

// load model, save it to another file. Then load this file and save again.
// compare second and third project file to be the same
TEST_F(mbapiModelTest, ModelLoadSaveProjectRoundTrip)
{
   std::unique_ptr<mbapi::Model> modelBase;
   modelBase.reset(new mbapi::Model());

   // load original project file
   ASSERT_EQ(ErrorHandler::NoError, modelBase->loadModelFromProjectFile(m_testProject));

   // Save a first copy 
   ASSERT_EQ(ErrorHandler::NoError, modelBase->saveModelToProjectFile("Project_case1.project3d"));

   std::unique_ptr<mbapi::Model> modelCase2;
   modelCase2.reset(new mbapi::Model());
   // load first copy
   ASSERT_EQ(ErrorHandler::NoError, modelCase2->loadModelFromProjectFile("Project_case1.project3d"));
   // save secon copy
   ASSERT_EQ(ErrorHandler::NoError, modelCase2->saveModelToProjectFile("Project_case2.project3d"));

   // compare first and second copies of project files to be the same
   ASSERT_TRUE(compareFiles("Project_case1.project3d", "Project_case2.project3d"));

   // clean files copy
   remove("Project_case1.project3d");
   remove("Project_case2.project3d");
}

// Test top crust heat production coefficient set/get methods
TEST_F(mbapiModelTest, BasementPropertySetting)
{
   mbapi::Model testModel;
   // load project file
   ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile(m_testProject));

   // check that BasementIoTable has only 1 record
   ASSERT_EQ(testModel.tableSize("BasementIoTbl"), 1);

   // Get bottom boundary condition model name. Must be "Fixed Temperature"
   std::string modelName = testModel.tableValueAsString("BasementIoTbl", 0, "BottomBoundaryModel");
   ASSERT_EQ(testModel.errorCode(), ErrorHandler::NoError);
   ASSERT_TRUE(modelName == "Fixed Temperature");

   // Get value for top crust heat production rate
   double topCrustHeatProd = testModel.tableValueAsDouble("BasementIoTbl", 0, "TopCrustHeatProd");
   ASSERT_EQ(ErrorHandler::NoError, testModel.errorCode());
   // must be 2.5
   ASSERT_NEAR(topCrustHeatProd, 2.5, eps);

   // change it value to 4.0
   ASSERT_EQ(ErrorHandler::NoError, testModel.setTableValue("BasementIoTbl", 0, "TopCrustHeatProd", 4.0));
   // and save as a new project
   ASSERT_EQ(ErrorHandler::NoError, testModel.saveModelToProjectFile("Project_basem_prop.project3d"));

   // reaload project to another model
   mbapi::Model modifModel;
   ASSERT_EQ(ErrorHandler::NoError, modifModel.loadModelFromProjectFile("Project_basem_prop.project3d"));

   // get value of top crust production rate
   topCrustHeatProd = modifModel.tableValueAsDouble("BasementIoTbl", 0, "TopCrustHeatProd");
   ASSERT_EQ(ErrorHandler::NoError, testModel.errorCode());

   // must be 4.0 (as it was set before)
   ASSERT_NEAR(topCrustHeatProd, 4.0, eps);

   // delete copy of the project
   remove("Project_basem_prop.project3d");
}

// Test set/get TOC in source rock lithology
TEST_F(mbapiModelTest, SourceRockTOCSettings)
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile(m_sourceRockTestProject));

   mbapi::SourceRockManager   & srMgr = testModel.sourceRockManager();
   mbapi::StratigraphyManager & stMgr = testModel.stratigraphyManager();

   // change TOC values to some others - 19, 70
   double ltocOld[] = { 70.2, 10.0 };
   double ltocNew[] = { 19.0, 70.0 };
   const char * lNames[] = { "Westphalian", "Lower Jurassic" };

   for (size_t i = 0; i < sizeof(ltocOld) / sizeof(double); ++i)
   {  // find correct source rock type
      mbapi::StratigraphyManager::LayerID lid = stMgr.layerID(lNames[i]);
      ASSERT_EQ(IsValueUndefined(lid), false);
      const std::vector<std::string> & layerSourceRocks = stMgr.sourceRockTypeName(lid);
      ASSERT_EQ(layerSourceRocks.size(), (i == 0 ? 2U : 1U));

      mbapi::SourceRockManager::SourceRockID sid = srMgr.findID(lNames[i], layerSourceRocks.front());
      ASSERT_EQ(IsValueUndefined(sid), false);

      // check what was set before
      double tocInFile = srMgr.tocIni(sid);
      ASSERT_EQ(ErrorHandler::NoError, srMgr.errorCode());
      ASSERT_NEAR(tocInFile, ltocOld[i], eps);

      // set the new value
      ASSERT_EQ(ErrorHandler::NoError, srMgr.setTOCIni(sid, ltocNew[i]));
   }

   // save as a new temporary project file
   ASSERT_EQ(ErrorHandler::NoError, testModel.saveModelToProjectFile("ChangedTOC.project3d"));

   // reload this new project file into other model
   mbapi::Model modifModel;
   ASSERT_EQ(ErrorHandler::NoError, modifModel.loadModelFromProjectFile("ChangedTOC.project3d"));

   mbapi::SourceRockManager   & srModMgr = modifModel.sourceRockManager();
   mbapi::StratigraphyManager & stModMgr = modifModel.stratigraphyManager();

   // check values for the TOC
   for (size_t i = 0; i < sizeof(ltocNew) / sizeof(double); ++i)
   {
      // find correct source rock type
      mbapi::StratigraphyManager::LayerID lid = stModMgr.layerID(lNames[i]);
      ASSERT_EQ(IsValueUndefined(lid), false);

      const std::vector<std::string> & layerSourceRocks = stModMgr.sourceRockTypeName(lid);
      ASSERT_EQ(layerSourceRocks.size(), (i == 0 ? 2U : 1U));

      mbapi::SourceRockManager::SourceRockID sid = srModMgr.findID(lNames[i], layerSourceRocks.front());
      ASSERT_EQ(IsValueUndefined(sid), false);

      // check if the new values are set
      double tocInFile = srModMgr.tocIni(sid);
      ASSERT_EQ(ErrorHandler::NoError, srModMgr.errorCode());
      ASSERT_NEAR(tocInFile, ltocNew[i], eps);
   }

   // delete temporary project file
   remove("ChangedTOC.project3d");
}

// Test set/get TOC in source rock lithology
TEST_F(mbapiModelTest, SourceRockHI_HCSettings)
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile(m_sourceRockTestProject));

   mbapi::SourceRockManager   & srMgr = testModel.sourceRockManager();
   mbapi::StratigraphyManager & stMgr = testModel.stratigraphyManager();

   const char * lNames[] = { "Westphalian", "Lower Jurassic" };

   for (size_t i = 0; i < 2; ++i)
   {  // find correct source rock type
      mbapi::StratigraphyManager::LayerID lid = stMgr.layerID(lNames[i]);
      ASSERT_EQ(IsValueUndefined(lid), false);

      // source rock must be active
      ASSERT_EQ(stMgr.isSourceRockActive(lid), true);

      // get source rock type
      const std::vector<std::string> & layerSourceRocks = stMgr.sourceRockTypeName(lid);

      // and find it id
      mbapi::SourceRockManager::SourceRockID sid = srMgr.findID(lNames[i], layerSourceRocks.front());
      ASSERT_EQ(IsValueUndefined(sid), false);

      double hiSR = srMgr.hiIni(sid);
      double hcSR = srMgr.hcIni(sid);

      // check mixing settings
      switch (i)
      {
      case 0:  // Wetphalian
      {
         ASSERT_EQ(stMgr.isSourceRockMixingEnabled(lid), true);
         ASSERT_EQ(layerSourceRocks.size(), 2U);
         ASSERT_NEAR(stMgr.sourceRockMixHI(lid), 700.0, eps);

         mbapi::SourceRockManager::SourceRockID sid2 = srMgr.findID(lNames[i], layerSourceRocks.back());
         ASSERT_EQ(IsValueUndefined(sid2), false);

         double hiSR2 = srMgr.hiIni(sid2);
         double hcSR2 = srMgr.hcIni(sid2);

         ASSERT_NEAR(hiSR, 94.364368, eps);
         ASSERT_NEAR(hiSR2, 472.068687, eps);

         ASSERT_NEAR(hcSR, 0.801, eps);
         ASSERT_NEAR(hcSR2, 1.25, eps);

         srMgr.setHCIni(sid2, 1.0);
         ASSERT_EQ(ErrorHandler::NoError, srMgr.errorCode());
         ASSERT_NEAR(srMgr.hiIni(sid2), 193.17523, eps);
      }
      break;

      case 1:  // Lower Jurassic
         ASSERT_EQ(stMgr.isSourceRockMixingEnabled(lid), false);
         ASSERT_EQ(layerSourceRocks.size(), 1U);
         ASSERT_NEAR(stMgr.sourceRockMixHI(lid), 0, eps);

         ASSERT_NEAR(hiSR, 472.068687, eps);
         ASSERT_NEAR(hcSR, 1.25, eps);

         srMgr.setHIIni(sid, 500.0);
         ASSERT_EQ(ErrorHandler::NoError, srMgr.errorCode());
         ASSERT_NEAR(srMgr.hcIni(sid), 1.261, eps);
         break;
      }
   }
}

TEST_F(mbapiModelTest, SnapshotManager)
{
   mbapi::Model testModel;
   // load project file
   ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile(m_testProject));

   // check that SnapshotIoTable has only 5 records
   ASSERT_EQ(testModel.snapshotManager().snapshotsNumber(), 5U);

   // add snapshot
   ASSERT_EQ(ErrorHandler::NoError, testModel.snapshotManager().requestMajorSnapshot(10.0));
   ASSERT_EQ(testModel.snapshotManager().snapshotsNumber(), 6U);

   ASSERT_NEAR(testModel.snapshotManager().time(1), 10.0, eps);
   ASSERT_EQ(testModel.snapshotManager().isMinor(1), false);
}


// Test set/get TOC in source rock lithology
TEST_F(mbapiModelTest, SourceRockPreAsphActEnergySettings)
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile(m_sourceRockTestProject));

   mbapi::SourceRockManager   & srMgr = testModel.sourceRockManager();
   mbapi::StratigraphyManager & stMgr = testModel.stratigraphyManager();

   // change preasphalt activation energy values to some others - 207, 211
   double lvalOld[] = { 206.0, 210.0 };
   double lvalNew[] = { 207.0, 211.0 };
   const char * lNames[] = { "Westphalian", "Lower Jurassic" };

   for (size_t i = 0; i < sizeof(lvalOld) / sizeof(double); ++i)
   {  // find correct source rock type
      mbapi::StratigraphyManager::LayerID lid = stMgr.layerID(lNames[i]);
      ASSERT_EQ(IsValueUndefined(lid), false);
      const std::vector<std::string> & layerSourceRocks = stMgr.sourceRockTypeName(lid);
      ASSERT_EQ(layerSourceRocks.size(), (i == 0 ? 2U : 1U));

      mbapi::SourceRockManager::SourceRockID sid = srMgr.findID(lNames[i], layerSourceRocks.front());
      ASSERT_EQ(IsValueUndefined(sid), false);

      // check what was set before
      double valInFile = srMgr.preAsphActEnergy(sid);
      ASSERT_EQ(ErrorHandler::NoError, srMgr.errorCode());
      ASSERT_NEAR(valInFile, lvalOld[i], eps);

      // set the new value
      ASSERT_EQ(ErrorHandler::NoError, srMgr.setPreAsphActEnergy(sid, lvalNew[i]));
   }

   // save as a new temporary project file
   ASSERT_EQ(ErrorHandler::NoError, testModel.saveModelToProjectFile("ChangedPreAsphEnergy.project3d"));

   // reload this new project file into other model
   mbapi::Model modifModel;
   ASSERT_EQ(ErrorHandler::NoError, modifModel.loadModelFromProjectFile("ChangedPreAsphEnergy.project3d"));

   mbapi::SourceRockManager   & srModMgr = modifModel.sourceRockManager();
   mbapi::StratigraphyManager & stModMgr = modifModel.stratigraphyManager();

   // check values for the TOC
   for (size_t i = 0; i < sizeof(lvalNew) / sizeof(double); ++i)
   {
      // find correct source rock type
      mbapi::StratigraphyManager::LayerID lid = stModMgr.layerID(lNames[i]);
      ASSERT_EQ(IsValueUndefined(lid), false);

      const std::vector<std::string> & layerSourceRocks = stModMgr.sourceRockTypeName(lid);
      ASSERT_EQ(layerSourceRocks.size(), (i == 0 ? 2U : 1U));

      mbapi::SourceRockManager::SourceRockID sid = srModMgr.findID(lNames[i], layerSourceRocks.front());
      ASSERT_EQ(IsValueUndefined(sid), false);

      // check if the new values are set
      double valInFile = srModMgr.preAsphActEnergy(sid);
      ASSERT_EQ(ErrorHandler::NoError, srModMgr.errorCode());
      ASSERT_NEAR(valInFile, lvalNew[i], eps);
   }

   // delete temporary project file
   remove("ChangedPreAsphEnergy.project3d");
}

TEST_F(mbapiModelTest, CopyLithologyTest)
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile(m_testProject));

   mbapi::LithologyManager & lthMgr = testModel.lithologyManager();

   mbapi::LithologyManager::LithologyID lid = lthMgr.findID("Std. Sandstone");
   ASSERT_FALSE(IsValueUndefined(lid));

   // first try to copy to existent lithology
   ASSERT_TRUE(IsValueUndefined(lthMgr.copyLithology(lid, "Crust")));
   ASSERT_EQ(lthMgr.errorCode(), ErrorHandler::AlreadyDefined);
   lthMgr.resetError();

   size_t lithNum = lthMgr.lithologiesIDs().size();

   mbapi::LithologyManager::LithologyID newLID = lthMgr.copyLithology(lid, "Std. Sandstone COPY");

   ASSERT_FALSE(IsValueUndefined(newLID));
   const std::string & newName = lthMgr.lithologyName(newLID);
   ASSERT_TRUE(newName == "Std. Sandstone COPY");
   ASSERT_EQ(lithNum + 1, lthMgr.lithologiesIDs().size());
}


TEST_F(mbapiModelTest, DeleteLithologyTest)
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile(m_testProject));

   mbapi::LithologyManager & lthMgr = testModel.lithologyManager();

   // First create a copy of lithology
   mbapi::LithologyManager::LithologyID lid = lthMgr.findID("Std. Sandstone");
   ASSERT_FALSE(IsValueUndefined(lid));
   mbapi::LithologyManager::LithologyID newLID = lthMgr.copyLithology(lid, "Std. Sandstone COPY");
   ASSERT_FALSE(IsValueUndefined(newLID));

   // then try to delete lithology which has references. Expecting ValidationError on this.
   ASSERT_EQ(ErrorHandler::ValidationError, lthMgr.deleteLithology(lid));

   // and now delete copy of lithology which has no any reference
   ASSERT_EQ(ErrorHandler::NoError, lthMgr.deleteLithology(newLID));

   newLID = lthMgr.findID("Std. Sandstone COPY");
   ASSERT_TRUE(IsValueUndefined(newLID));
}

TEST_F(mbapiModelTest, DeleteDuplicatedLithologyTest)
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile(m_dupLithologyTestProject));

   mbapi::LithologyManager & lthMgr = testModel.lithologyManager();

   size_t lithNum = lthMgr.lithologiesIDs().size();

   // project file already has duplicated lithologies. Clean them
   ASSERT_EQ(lithNum, 14U);
   ASSERT_EQ(ErrorHandler::NoError, lthMgr.cleanDuplicatedLithologies());

   // check that 5 lithologies were deleted
   lithNum = lthMgr.lithologiesIDs().size();
   ASSERT_EQ(lithNum, 9U);
}


TEST_F(mbapiModelTest, GetPermeabilityModelParametersTest)
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile(m_lithologyTestProject));

   mbapi::LithologyManager & lthMgr = testModel.lithologyManager();

   // Check Sands permeability
   mbapi::LithologyManager::LithologyID lid = lthMgr.findID("Std. Sandstone");
   ASSERT_FALSE(IsValueUndefined(lid));

   mbapi::LithologyManager::PermeabilityModel permModel;
   std::vector<double> modelPrms;
   std::vector<double> mpPor;
   std::vector<double> mpPerm;

   ASSERT_EQ(ErrorHandler::NoError, lthMgr.permeabilityModel(lid, permModel, modelPrms, mpPor, mpPerm));
   ASSERT_EQ(permModel, mbapi::LithologyManager::PermSandstone);
   ASSERT_EQ(modelPrms.size(), 3U);
   ASSERT_EQ(mpPor.size(), 0U);
   ASSERT_EQ(mpPerm.size(), 0U);
   ASSERT_NEAR(modelPrms[0], 1.0, eps);
   ASSERT_NEAR(modelPrms[1], 6000.0, eps);
   ASSERT_NEAR(modelPrms[2], 1.5, eps);

   // Check Shales permeability
   lid = lthMgr.findID("Std. Shale");
   ASSERT_FALSE(IsValueUndefined(lid));

   ASSERT_EQ(ErrorHandler::NoError, lthMgr.permeabilityModel(lid, permModel, modelPrms, mpPor, mpPerm));
   ASSERT_EQ(permModel, mbapi::LithologyManager::PermMudstone);
   ASSERT_EQ(modelPrms.size(), 4U);
   ASSERT_EQ(mpPor.size(), 0U);
   ASSERT_EQ(mpPerm.size(), 0U);
   ASSERT_NEAR(modelPrms[0], 1.0, eps);
   ASSERT_NEAR(modelPrms[1], 0.01, eps);
   ASSERT_NEAR(modelPrms[2], 1.5, eps);
   ASSERT_NEAR(modelPrms[3], 0.01, eps);

   // Check Nones permeability
   lid = lthMgr.findID("Crust");
   ASSERT_FALSE(IsValueUndefined(lid));

   ASSERT_EQ(ErrorHandler::NoError, lthMgr.permeabilityModel(lid, permModel, modelPrms, mpPor, mpPerm));
   ASSERT_EQ(permModel, mbapi::LithologyManager::PermNone);
   ASSERT_EQ(modelPrms.size(), 0U);
   ASSERT_EQ(mpPor.size(), 0U);
   ASSERT_EQ(mpPerm.size(), 0U);

   // Check Impermeable permeability
   lid = lthMgr.findID("Standard Ice");
   ASSERT_FALSE(IsValueUndefined(lid));

   ASSERT_EQ(ErrorHandler::NoError, lthMgr.permeabilityModel(lid, permModel, modelPrms, mpPor, mpPerm));
   ASSERT_EQ(permModel, mbapi::LithologyManager::PermImpermeable);
   ASSERT_EQ(modelPrms.size(), 0U);
   ASSERT_EQ(mpPor.size(), 0U);
   ASSERT_EQ(mpPerm.size(), 0U);

   // Check Multipoint permeability
   lid = lthMgr.findID("SM. Sandstone");
   ASSERT_FALSE(IsValueUndefined(lid));

   ASSERT_EQ(ErrorHandler::NoError, lthMgr.permeabilityModel(lid, permModel, modelPrms, mpPor, mpPerm));
   ASSERT_EQ(permModel, mbapi::LithologyManager::PermMultipoint);
   ASSERT_EQ(modelPrms.size(), 1U);
   ASSERT_NEAR(modelPrms[0], 1.0, eps);
   ASSERT_EQ(mpPor.size(), 2U);
   ASSERT_EQ(mpPerm.size(), 2U);
   ASSERT_NEAR(mpPor[0], 5.0, eps);
   ASSERT_NEAR(mpPor[1], 60.0, eps);
   ASSERT_NEAR(mpPerm[0], 0.3, eps);
   ASSERT_NEAR(mpPerm[1], 8.6, eps);
}



TEST_F(mbapiModelTest, SetPermeabilityModelParametersTest)
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile(m_lithologyTestProject));

   mbapi::LithologyManager & lthMgr = testModel.lithologyManager();

   // Check Sands permeability
   mbapi::LithologyManager::LithologyID lid = lthMgr.findID("Crust");
   ASSERT_FALSE(IsValueUndefined(lid));

   // change model from None to Impermeable
   mbapi::LithologyManager::PermeabilityModel permModel = mbapi::LithologyManager::PermImpermeable;
   std::vector<double> modelPrms;
   std::vector<double> mpPor;
   std::vector<double> mpPerm;

   ASSERT_EQ(ErrorHandler::NoError, lthMgr.setPermeabilityModel(lid, permModel, modelPrms, mpPor, mpPerm));

   // change from Impermeable to None
   lid = lthMgr.findID("Standard Ice");
   ASSERT_FALSE(IsValueUndefined(lid));

   permModel = mbapi::LithologyManager::PermNone;
   ASSERT_EQ(ErrorHandler::NoError, lthMgr.setPermeabilityModel(lid, permModel, modelPrms, mpPor, mpPerm));

   // change a bit coeff for Mudstone
   lid = lthMgr.findID("Std. Shale");
   ASSERT_FALSE(IsValueUndefined(lid));

   permModel = mbapi::LithologyManager::PermMudstone;
   modelPrms.push_back(1.0);  // anisothropic coeff
   modelPrms.push_back(0.03); // depo por
   modelPrms.push_back(1.55); // incr coeff
   modelPrms.push_back(0.02); // decr coeff

   ASSERT_EQ(ErrorHandler::NoError, lthMgr.setPermeabilityModel(lid, permModel, modelPrms, mpPor, mpPerm));

   // change Sandstone permeability
   lid = lthMgr.findID("Std. Sandstone");
   ASSERT_FALSE(IsValueUndefined(lid));

   permModel = mbapi::LithologyManager::PermSandstone;
   modelPrms.resize(1);
   modelPrms.push_back(7000.0); // depo por
   modelPrms.push_back(1.45); // incr coeff

   ASSERT_EQ(ErrorHandler::NoError, lthMgr.setPermeabilityModel(lid, permModel, modelPrms, mpPor, mpPerm));

   // change Multipoint permeability
   lid = lthMgr.findID("SM. Sandstone");
   ASSERT_FALSE(IsValueUndefined(lid));
   permModel = mbapi::LithologyManager::PermMultipoint;

   modelPrms.resize(1);
   mpPor.push_back(10.0);
   mpPor.push_back(20.0);
   mpPor.push_back(30.0);

   mpPerm.push_back(2.0);
   mpPerm.push_back(3.0);
   mpPerm.push_back(4.0);

   ASSERT_EQ(ErrorHandler::NoError, lthMgr.setPermeabilityModel(lid, permModel, modelPrms, mpPor, mpPerm));

   // do round trip - save/load project file and compare parameters
   std::string newProjName("mod_");
   newProjName += m_lithologyTestProject;

   ASSERT_EQ(ErrorHandler::NoError, lthMgr.setPermeabilityModel(lid, permModel, modelPrms, mpPor, mpPerm));
   ASSERT_EQ(ErrorHandler::NoError, testModel.saveModelToProjectFile(newProjName.c_str()));

   mpPor.clear();
   mpPerm.clear();

   mbapi::Model checkTestModel;
   ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile(newProjName.c_str()));

   lid = lthMgr.findID("Crust");
   ASSERT_FALSE(IsValueUndefined(lid));

   ASSERT_EQ(ErrorHandler::NoError, lthMgr.permeabilityModel(lid, permModel, modelPrms, mpPor, mpPerm));
   ASSERT_EQ(permModel, mbapi::LithologyManager::PermImpermeable);
   ASSERT_EQ(modelPrms.size(), 0U);
   ASSERT_EQ(mpPor.size(), 0U);
   ASSERT_EQ(mpPerm.size(), 0U);

   lid = lthMgr.findID("Standard Ice");
   ASSERT_FALSE(IsValueUndefined(lid));

   ASSERT_EQ(ErrorHandler::NoError, lthMgr.permeabilityModel(lid, permModel, modelPrms, mpPor, mpPerm));
   ASSERT_EQ(permModel, mbapi::LithologyManager::PermNone);
   ASSERT_EQ(modelPrms.size(), 0U);
   ASSERT_EQ(mpPor.size(), 0U);
   ASSERT_EQ(mpPerm.size(), 0U);

   lid = lthMgr.findID("Std. Shale");
   ASSERT_FALSE(IsValueUndefined(lid));

   ASSERT_EQ(ErrorHandler::NoError, lthMgr.permeabilityModel(lid, permModel, modelPrms, mpPor, mpPerm));
   ASSERT_EQ(permModel, mbapi::LithologyManager::PermMudstone);
   ASSERT_EQ(modelPrms.size(), 4U);
   ASSERT_EQ(mpPor.size(), 0U);
   ASSERT_EQ(mpPerm.size(), 0U);
   ASSERT_NEAR(modelPrms[0], 1.0, eps);
   ASSERT_NEAR(modelPrms[1], 0.03, eps);
   ASSERT_NEAR(modelPrms[2], 1.55, eps);
   ASSERT_NEAR(modelPrms[3], 0.02, eps);

   lid = lthMgr.findID("Std. Sandstone");
   ASSERT_FALSE(IsValueUndefined(lid));

   ASSERT_EQ(ErrorHandler::NoError, lthMgr.permeabilityModel(lid, permModel, modelPrms, mpPor, mpPerm));
   ASSERT_EQ(permModel, mbapi::LithologyManager::PermSandstone);
   ASSERT_EQ(modelPrms.size(), 3U);
   ASSERT_EQ(mpPor.size(), 0U);
   ASSERT_EQ(mpPerm.size(), 0U);
   ASSERT_NEAR(modelPrms[0], 1.0, eps);
   ASSERT_NEAR(modelPrms[1], 7000.0, eps);
   ASSERT_NEAR(modelPrms[2], 1.45, eps);

   // Check Multipoint permeability
   lid = lthMgr.findID("SM. Sandstone");
   ASSERT_FALSE(IsValueUndefined(lid));

   ASSERT_EQ(ErrorHandler::NoError, lthMgr.permeabilityModel(lid, permModel, modelPrms, mpPor, mpPerm));
   ASSERT_EQ(permModel, mbapi::LithologyManager::PermMultipoint);
   ASSERT_EQ(modelPrms.size(), 1U);
   ASSERT_NEAR(modelPrms[0], 1.0, eps);
   ASSERT_EQ(mpPor.size(), 3U);
   ASSERT_EQ(mpPerm.size(), 3U);
   ASSERT_NEAR(mpPor[0], 10.0, eps);
   ASSERT_NEAR(mpPor[1], 20.0, eps);
   ASSERT_NEAR(mpPor[2], 30.0, eps);
   ASSERT_NEAR(mpPerm[0], 2.0, eps);
   ASSERT_NEAR(mpPerm[1], 3.0, eps);
   ASSERT_NEAR(mpPerm[2], 4.0, eps);
}


TEST_F(mbapiModelTest, PorosityModelParametersTest)
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile(m_lithologyTestProject));

   mbapi::LithologyManager & lthMgr = testModel.lithologyManager();

   // Check get Exponential porosity model
   mbapi::LithologyManager::LithologyID lid1 = lthMgr.findID("Std. Sandstone");
   ASSERT_FALSE(IsValueUndefined(lid1));

   mbapi::LithologyManager::PorosityModel porModel1;
   std::vector<double> modelPrms1;

   ASSERT_EQ(ErrorHandler::NoError, lthMgr.porosityModel(lid1, porModel1, modelPrms1));
   ASSERT_EQ(porModel1, mbapi::LithologyManager::PorExponential);
   ASSERT_EQ(modelPrms1.size(), 3U);
   ASSERT_NEAR(modelPrms1[0], 48.0, eps);
   ASSERT_NEAR(modelPrms1[1], 3.22, eps);
   ASSERT_NEAR(modelPrms1[2], 0.0, eps);

   // Check get Soil mechanics porosity model
   mbapi::LithologyManager::LithologyID lid2 = lthMgr.findID("SM.Mudst.40%Clay");
   ASSERT_FALSE(IsValueUndefined(lid2));

   mbapi::LithologyManager::PorosityModel porModel2;
   std::vector<double> modelPrms2;

   ASSERT_EQ(ErrorHandler::NoError, lthMgr.porosityModel(lid2, porModel2, modelPrms2));
   ASSERT_EQ(porModel2, mbapi::LithologyManager::PorSoilMechanics);
   ASSERT_EQ(modelPrms2.size(), 2U);
   ASSERT_NEAR(modelPrms2[0], 56.32, eps);
   ASSERT_NEAR(modelPrms2[1], 0.1988, eps);

   mbapi::LithologyManager::PorosityModel porModel3;
   std::vector<double> modelPrms3;

   // Check set Soil mechanics porosity model
   ASSERT_EQ(ErrorHandler::NoError, lthMgr.setPorosityModel(lid1, porModel2, modelPrms2));
   ASSERT_EQ(ErrorHandler::NoError, lthMgr.porosityModel(lid1, porModel3, modelPrms3));
   ASSERT_EQ(porModel3, porModel2);
   ASSERT_EQ(modelPrms3.size(), modelPrms2.size());
   ASSERT_NEAR(modelPrms3[0], modelPrms2[0], eps);
   ASSERT_NEAR(modelPrms3[1], modelPrms2[1], eps);

   // Check set Exponential porosity model
   ASSERT_EQ(ErrorHandler::NoError, lthMgr.setPorosityModel(lid2, porModel1, modelPrms1));
   ASSERT_EQ(ErrorHandler::NoError, lthMgr.porosityModel(lid2, porModel3, modelPrms3));
   ASSERT_EQ(porModel3, porModel1);
   ASSERT_EQ(modelPrms3.size(), modelPrms1.size());
   ASSERT_NEAR(modelPrms3[0], modelPrms1[0], eps);
   ASSERT_NEAR(modelPrms3[1], modelPrms1[1], eps);
   ASSERT_NEAR(modelPrms3[2], modelPrms1[2], eps);
}

TEST_F(mbapiModelTest, GetSetLithologyDescriptionTest)
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile(m_lithologyTestProject));

   mbapi::LithologyManager & lthMgr = testModel.lithologyManager();

   // Get a lithology description
   mbapi::LithologyManager::LithologyID lid = lthMgr.findID("Std. Sandstone");
   ASSERT_EQ("Standard Sandstone", lthMgr.getDescription(lid));
   // Set a lithology description
   lthMgr.setDescription(lid, lthMgr.getDescription(lid) + " (upgraded to double exponential model)");
   ASSERT_EQ("Standard Sandstone (upgraded to double exponential model)", lthMgr.getDescription(lid));
}


TEST_F(mbapiModelTest, MapsManagerCopyMapTest)
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile(m_mapsTestProject));
   std::string mapName = "MAP-1076770443-4";

   // clean any previous runs result
   if (ibs::FilePath("MAP-1076770443-4_copy.HDF").exists()) { ibs::FilePath("MAP-1076770443-4_copy.HDF").remove(); }
   if (ibs::FilePath("Test.HDF").exists()) { ibs::FilePath("Test.HDF").remove(); }
   if (ibs::FilePath("MapsTest1.project3d").exists()) { ibs::FilePath("MapsTest1.project3d").remove(); }
   if (ibs::FilePath("MapsTest2.project3d").exists()) { ibs::FilePath("MapsTest2.project3d").remove(); }

   mbapi::MapsManager & mm = testModel.mapsManager();

   const std::vector<mbapi::MapsManager::MapID> & ids = mm.mapsIDs();
   ASSERT_EQ(ids.size(), 8U); // number of maps in GridMapIoTbl

   mbapi::MapsManager::MapID id = mm.findID(mapName);
   ASSERT_EQ(id, 6U); // given map is 7th in the list

   double minV, maxV;
   ASSERT_EQ(ErrorHandler::NoError, mm.mapValuesRange(id, minV, maxV));

   ASSERT_NEAR(minV, 2288.0, eps);
   ASSERT_NEAR(maxV, 6434.0, eps);

   mbapi::MapsManager::MapID nid = mm.copyMap(id, mapName + "_copy");

   ASSERT_EQ(nid, 8U); // the new map is at the end of the list
   ASSERT_EQ(mm.mapsIDs().size(), 9U); // now maps in GridMapIoTbl are 9

   // min/max values in the copy are the same
   mm.mapValuesRange(nid, minV, maxV);

   ASSERT_NEAR(minV, 2288.0, eps);
   ASSERT_NEAR(maxV, 6434.0, eps);

   double coeff = 3000 / maxV;
   ASSERT_EQ(ErrorHandler::NoError, mm.scaleMap(nid, coeff));

   // min/max values in the copy are as given for rescale
   mm.mapValuesRange(nid, minV, maxV);

   ASSERT_NEAR(minV, 2288.0 * coeff, eps);
   ASSERT_NEAR(maxV, 3000.0, eps);


   // If a file already exist the maps will be appended to that file
   ASSERT_EQ(ErrorHandler::OutOfRangeValue, mm.saveMapToHDF(nid, "/tmp/Inputs.HDF", 0)); // attempt to save in a place different from the project location should fail

   ASSERT_EQ(ErrorHandler::NoError, mm.saveMapToHDF(nid, "", 0)); // file name will be generated from map name
   ASSERT_EQ(true, ibs::FilePath(mapName + "_copy.HDF").exists()); // file was written

   ASSERT_EQ(ErrorHandler::NoError, testModel.saveModelToProjectFile("MapsTest1.project3d"));
   {
      mbapi::Model tmpModel;
      tmpModel.loadModelFromProjectFile("MapsTest1.project3d");

      const std::vector<mbapi::MapsManager::MapID> & tids = tmpModel.mapsManager().mapsIDs();
      ASSERT_EQ(tids.size(), 9U);
      ASSERT_EQ(tmpModel.tableValueAsString("GridMapIoTbl", 8, "MapName"), std::string("MAP-1076770443-4_copy"));
      ASSERT_EQ(tmpModel.tableValueAsString("GridMapIoTbl", 8, "MapFileName"), std::string("MAP-1076770443-4_copy.HDF"));
      ASSERT_EQ(tmpModel.tableValueAsString("GridMapIoTbl", 8, "MapType"), std::string("HDF5"));
   }
   ibs::FilePath("MapsTest1.project3d").remove();

   ASSERT_EQ(ErrorHandler::NoError, mm.saveMapToHDF(nid, "Test.HDF", 0)); // file name will be generated from map name
   ASSERT_EQ(true, ibs::FilePath("Test.HDF").exists()); // file was written

   ASSERT_EQ(ErrorHandler::NoError, testModel.saveModelToProjectFile("MapsTest2.project3d"));
   {
      mbapi::Model tmpModel;
      tmpModel.loadModelFromProjectFile("MapsTest2.project3d");

      const std::vector<mbapi::MapsManager::MapID> & tids = tmpModel.mapsManager().mapsIDs();
      ASSERT_EQ(tids.size(), 9U);
      ASSERT_EQ(tmpModel.tableValueAsString("GridMapIoTbl", 8, "MapName"), std::string("MAP-1076770443-4_copy"));
      ASSERT_EQ(tmpModel.tableValueAsString("GridMapIoTbl", 8, "MapFileName"), std::string("Test.HDF"));
      ASSERT_EQ(tmpModel.tableValueAsString("GridMapIoTbl", 8, "MapType"), std::string("HDF5"));
   }
   ibs::FilePath("MapsTest2.project3d").remove();
}

TEST_F(mbapiModelTest, MapsManagerNNInterpolation)
{
   // set the folder where to save the files
   ibs::FilePath    masterResults(".");
   std::string      casaResultsFile("CasaModel_Results.HDF");
   masterResults << casaResultsFile;

   // clean any previous runs result
   if (masterResults.exists()) { masterResults.remove(); }
   if (ibs::FilePath("NNTesting2.project3d").exists()) { ibs::FilePath("NNTesting2.project3d").remove(); }

   // load test project
   mbapi::Model testModel;
   ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile(m_nnTestProject));

   // read lithofractions from file
   std::ifstream NNInputLithofractions("NNInputLithofractions");
   double xTemp;
   double yTemp;
   double lf1Temp;
   double lf2Temp;
   double lf3Temp;
   std::vector<double> xin;
   std::vector<double> yin;
   std::vector<double> lf1;
   std::vector<double> lf2;
   std::vector<double> lf3;

   while (NNInputLithofractions.good())
   {
      NNInputLithofractions >> xTemp;
      NNInputLithofractions >> yTemp;
      NNInputLithofractions >> lf1Temp;
      NNInputLithofractions >> lf2Temp;
      NNInputLithofractions >> lf3Temp;
      xin.push_back(xTemp);
      yin.push_back(yTemp);
      lf1.push_back(lf1Temp);
      lf2.push_back(lf2Temp);
      lf3.push_back(lf3Temp);
   }
   NNInputLithofractions.close();

   std::vector<double> xout;
   std::vector<double> yout;
   std::vector<double> rpInt;
   std::vector<double> r13Int;

   testModel.interpolateLithoFractions(xin, yin, lf1, lf2, lf3, xout, yout, rpInt, r13Int);

   // check the number of points is correct
   ASSERT_EQ(xout.size(), 6527U);
   ASSERT_EQ(yout.size(), 6527U);
   ASSERT_EQ(rpInt.size(), 6527U);
   ASSERT_EQ(r13Int.size(), 6527U);

   // check the interpolated values are equal to those generated by the original prototype
   std::ifstream NNInt("NNInt");
   double rpTemp;
   double r13Temp;
   for (size_t i = 0; i < xout.size(); ++i)
   {
      NNInt >> xTemp;
      NNInt >> yTemp;
      NNInt >> rpTemp;
      NNInt >> r13Temp;
      ASSERT_NEAR(xTemp, xout[i], eps);
      ASSERT_NEAR(yTemp, yout[i], eps);
      ASSERT_NEAR(rpTemp, rpInt[i], eps);
      ASSERT_NEAR(r13Temp, r13Int[i], eps);
   }
   NNInt.close();

   // back transform the lithofractions
   std::vector<double> lf1CorrInt;
   std::vector<double> lf2CorrInt;
   std::vector<double> lf3CorrInt;
   testModel.backTransformLithoFractions(rpInt, r13Int, lf1CorrInt, lf2CorrInt, lf3CorrInt);

   // check the back transformation is correct, as in the original prototype
   std::ifstream NNbt("NNbt");
   double lf1CorrTemp;
   double lf2CorrTemp;
   for (size_t i = 0; i < lf1CorrInt.size(); ++i)
   {
      NNbt >> lf1CorrTemp;
      NNbt >> lf2CorrTemp;
      ASSERT_NEAR(lf1CorrTemp, lf1CorrInt[i], 1e-4);
      ASSERT_NEAR(lf2CorrTemp, lf2CorrInt[i], 1e-4);
   }
   NNbt.close();

   // save the backtransformed lithofractions in a temporary model
   ASSERT_EQ(ErrorHandler::NoError, testModel.saveModelToProjectFile("NNTesting2.project3d"));
   {
      mbapi::Model tmpModel;
      tmpModel.loadModelFromProjectFile("NNTesting2.project3d");

      // get the maps manager
      mbapi::MapsManager & mapsMgr = tmpModel.mapsManager();

      // get the stratigraphy manager
      mbapi::StratigraphyManager & strMgr = tmpModel.stratigraphyManager();

      // get the layer ID
      mbapi::StratigraphyManager::LayerID lid = strMgr.layerID("Rotliegend");

      // generate the maps
      std::string  correctFirstLithoFractionMap("5_percent_1");
      std::string  correctSecondLithoFractionMap("5_percent_2");
      size_t mapSeqNbr = Utilities::Numerical::NoDataIDValue;

      // first map, produce the map and update GridmapIoTbl
      mbapi::MapsManager::MapID id = mapsMgr.generateMap("StratIoTbl", correctFirstLithoFractionMap, lf1CorrInt, mapSeqNbr, masterResults.path());
      ASSERT_FALSE(IsValueUndefined(id));

      // second map, produce the map and update GridmapIoTbl
      id = mapsMgr.generateMap("StratIoTbl", correctSecondLithoFractionMap, lf2CorrInt, mapSeqNbr, masterResults.path());
      ASSERT_FALSE(IsValueUndefined(id));

      ASSERT_EQ(ErrorHandler::NoError, strMgr.setLayerLithologiesPercentageMaps(lid, correctFirstLithoFractionMap, correctSecondLithoFractionMap));

      const std::vector<mbapi::MapsManager::MapID> & tids = tmpModel.mapsManager().mapsIDs();
      ASSERT_EQ(tids.size(), 28U);

      ASSERT_EQ(tmpModel.tableValueAsString("GridMapIoTbl", 26, "MapName"), std::string(correctFirstLithoFractionMap));
      ASSERT_EQ(tmpModel.tableValueAsString("GridMapIoTbl", 27, "MapName"), std::string(correctSecondLithoFractionMap));

      ASSERT_EQ(tmpModel.tableValueAsString("GridMapIoTbl", 26, "MapFileName"), casaResultsFile);
      ASSERT_EQ(tmpModel.tableValueAsString("GridMapIoTbl", 27, "MapFileName"), casaResultsFile);

      ASSERT_EQ(tmpModel.tableValueAsString("StratIoTbl", 5, "Percent1Grid"), correctFirstLithoFractionMap);
      ASSERT_EQ(tmpModel.tableValueAsString("StratIoTbl", 5, "Percent2Grid"), correctSecondLithoFractionMap);
   }

   ibs::FilePath("NNTesting2.project3d").remove();
   masterResults.remove();
}

TEST_F(mbapiModelTest, FluidManager)
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile(m_fluidTestProject));

   mbapi::FluidManager & flMgr = testModel.fluidManager();


   //get fluid ids
   auto fluids = flMgr.getFluidsID();

   size_t actualTableSize = 5;
   //check whether all entries in FluidTypeIOTbl were read
   ASSERT_EQ(actualTableSize, fluids.size());


   {
      std::vector<std::string> actualFluidNames = { "Std. Water","Std. Marine Water","Std. Hyper Saline Water","Std. Sea Water","NVG_Water" };

      std::string fluidName;

      //Check whether all names are being read correctlty
      for (auto flId : fluids)
      {
         flMgr.getFluidName(flId, fluidName);
         EXPECT_EQ(actualFluidNames[flId], fluidName);
      }
   }

   //check whether user defined flag can be read and modified
   {
      mbapi::FluidManager::FluidID id;
      std::vector<int> actualUserDefined = { 0,0,0,0,1 };

      int myUserDefined;

      //Check whether all names are being read correctlty
      for (auto flId : fluids)
      {
         flMgr.getUserDefined(flId, myUserDefined);
         EXPECT_EQ(actualUserDefined[flId], myUserDefined);
      }

      id = 1;
      flMgr.setUserDefined(id, 1);
      flMgr.getUserDefined(id, myUserDefined);
      EXPECT_EQ(1, myUserDefined);
      flMgr.setUserDefined(id, 0);

      id = 4;
      flMgr.setUserDefined(id, 0);
      flMgr.getUserDefined(id, myUserDefined);
      EXPECT_EQ(0, myUserDefined);
      flMgr.setUserDefined(id, 1);
   }

   //check whether fluid description can be read and modified
   {
      std::vector<std::string> actualDescription = { "KSEPL's Standard Water","KSEPL's Standard Marine Water",
         "KSEPL's Standard Ultra Marine Water","KSEPL's Standard Sea Water","KSEPL's Standard Water" };
      std::string myDescription;

      //Check whether all names are being read correctlty
      for (auto flId : fluids)
      {
         flMgr.getDescription(flId, myDescription);
         EXPECT_EQ(actualDescription[flId], myDescription);
      }

      mbapi::FluidManager::FluidID id = 1;
      flMgr.setDescription(id, "KSEPL's Deprecated Marine Water");
      flMgr.getDescription(id, myDescription);
      EXPECT_EQ("KSEPL's Deprecated Marine Water", myDescription);
      flMgr.setDescription(id, "KSEPL's Standard Marine Water");


   }
   //check whether density model and value can be read and modified
   {
      mbapi::FluidManager::FluidID id;
      mbapi::FluidManager::FluidDensityModel model1, model2;
      double density1, density2;

      id = 0;
      flMgr.densityModel(id, model1, density1);
      EXPECT_EQ(mbapi::FluidManager::FluidDensityModel::Calculated, model1);
      EXPECT_NEAR(1000.0, density1, eps);

      id = 4;
      flMgr.densityModel(id, model1, density1);
      EXPECT_EQ(mbapi::FluidManager::FluidDensityModel::Constant, model1);
      EXPECT_NEAR(1000.0, density1, eps);
      density1 = density1 * 10.0;
      flMgr.setDensityModel(id, mbapi::FluidManager::FluidDensityModel::Calculated, density1);

      flMgr.densityModel(id, model2, density2);
      EXPECT_EQ(mbapi::FluidManager::FluidDensityModel::Calculated, model2);
      EXPECT_NEAR(density1, density2, eps);

      density1 = density1 / 10.0;
      flMgr.setDensityModel(id, mbapi::FluidManager::FluidDensityModel::Constant, density1);
   }

   //check whether seismic velocity model and value can be read and modified
   {
      mbapi::FluidManager::FluidID id;
      mbapi::FluidManager::CalculationModel model1, model2;
      double seismicVelocity1, seismicVelocity2;

      id = 0;
      flMgr.seismicVelocityModel(id, model1, seismicVelocity1);
      EXPECT_EQ(mbapi::FluidManager::CalculationModel::CalculatedModel, model1);
      EXPECT_NEAR(1500.0, seismicVelocity1, eps);

      id = 4;
      flMgr.seismicVelocityModel(id, model1, seismicVelocity1);
      EXPECT_EQ(mbapi::FluidManager::CalculationModel::ConstantModel, model1);
      EXPECT_NEAR(1500.0, seismicVelocity1, eps);
      seismicVelocity1 = seismicVelocity1 * 10.0;
      flMgr.setSeismicVelocityModel(id, mbapi::FluidManager::CalculationModel::CalculatedModel, seismicVelocity1);

      flMgr.seismicVelocityModel(id, model2, seismicVelocity2);
      EXPECT_EQ(mbapi::FluidManager::CalculationModel::CalculatedModel, model2);
      EXPECT_NEAR(seismicVelocity1, seismicVelocity2, eps);

      seismicVelocity1 = seismicVelocity1 / 10.0;
      flMgr.setSeismicVelocityModel(id, mbapi::FluidManager::CalculationModel::ConstantModel, seismicVelocity1);
   }
}

TEST_F(mbapiModelTest, ReservoirManager)
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile(m_reservoirTestProject));

   mbapi::ReservoirManager & resMgr = testModel.reservoirManager();

   //get reservoir ids
   auto res = resMgr.getReservoirsID();

   size_t actualTableSize = 2;
   //check whether all entries in ResevoirIoTbl were read
   ASSERT_EQ(actualTableSize, res.size());

   std::vector<std::string> actualReservoirNames = { "reservoir 1","reservoir 0" };

   std::string reservoirName;
   //   mbapi::ReservoirManager & resMgrImpl = testModel.reservoirManager();
      //Check whether all names are being read correctlty

   for (auto resId : res)
   {
      resMgr.getResName(resId, reservoirName);
      ASSERT_EQ(actualReservoirNames[resId], reservoirName);
   }
   double resCapacity;
   int bioDegradInd;
   int oilToGasCrackingInd;
   int blockingInd;
   double blockingPermeability;
   size_t resId = 0;
   resMgr.getResCapacity(resId, resCapacity);
   EXPECT_NEAR(resCapacity, 500000, eps);
   resMgr.getResBioDegradInd(resId, bioDegradInd);
   EXPECT_EQ(bioDegradInd, 1);
   resMgr.getResOilToGasCrackingInd(resId, oilToGasCrackingInd);
   EXPECT_EQ(oilToGasCrackingInd, 1);
   resMgr.getResBlockingInd(resId, blockingInd);
   EXPECT_EQ(blockingInd, 0);
   resMgr.getResBlockingPermeability(resId, blockingPermeability);
   EXPECT_NEAR(blockingPermeability, 1e-09, eps);
   //
   resId = 1;
   resMgr.getResCapacity(resId, resCapacity);
   EXPECT_NEAR(resCapacity, 400000, eps);
   resMgr.getResBioDegradInd(resId, bioDegradInd);
   EXPECT_EQ(bioDegradInd, 0);
   resMgr.getResOilToGasCrackingInd(resId, oilToGasCrackingInd);
   EXPECT_EQ(oilToGasCrackingInd, 1);
   resMgr.getResBlockingInd(resId, blockingInd);
   EXPECT_EQ(blockingInd, 0);
   resMgr.getResBlockingPermeability(resId, blockingPermeability);
   EXPECT_NEAR(blockingPermeability, 1e-09, eps);
   //checks entries set in ReservoirIoTble
   resCapacity = 400000;
   bioDegradInd = 1;
   oilToGasCrackingInd = 1;
   blockingInd = 0;
   blockingPermeability = 1e-9;
   resId = 0;
   resMgr.setResCapacity(resId, resCapacity);
   resMgr.setResBioDegradInd(resId, bioDegradInd);
   resMgr.setResOilToGasCrackingInd(resId, oilToGasCrackingInd);
   resMgr.setResBlockingInd(resId, blockingInd);
   resMgr.setResBlockingPermeability(resId, blockingPermeability);
   resMgr.getResCapacity(resId, resCapacity);
   EXPECT_NEAR(resCapacity, 400000, eps);
   resMgr.getResBioDegradInd(resId, bioDegradInd);
   EXPECT_EQ(bioDegradInd, 1);
   resMgr.getResOilToGasCrackingInd(resId, oilToGasCrackingInd);
   EXPECT_EQ(oilToGasCrackingInd, 1);
   resMgr.getResBlockingInd(resId, blockingInd);
   EXPECT_EQ(blockingInd, 0);
   resMgr.getResBlockingPermeability(resId, blockingPermeability);
   EXPECT_NEAR(blockingPermeability, 1e-9, eps);
}

TEST_F(mbapiModelTest, BiodegradeManager)
{
   mbapi::Model testModel;

   // load test project
   ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile(m_fluidTestProject));

   mbapi::BiodegradeManager & bioDegMgr = testModel.biodegradeManager();

   std::map<std::string, double> mapBioDegConstants = {
      { "MaxBioTemp", 65.0 },
      { "TempConstant", 70.0 },
      { "TimeConstant", 0.2 },
      { "BioRate", 0.3 },
      { "C1_BioFactor", 0.0008 },
      { "C2_BioFactor", 0.0005 },
      { "C3_BioFactor", 0.0003 },
      { "C4_BioFactor", 0.0008 },
      { "C5_BioFactor", 0.001 },
      { "N2_BioFactor", 0.001 },
      { "COx_BioFactor", 0.0 },
      { "C6_14Aro_BioFactor", 0.0015 },
      { "C6_14Sat_BioFactor", 0.001 },
      { "C15Aro_BioFactor", 0.03 },
      { "C15Sat_BioFactor", 0.0009 },
      { "resins_BioFactor", 0.05 },
      { "asphaltenes_BioFactor", 0.1 }
   };

   for (auto& itr : mapBioDegConstants) {
      double valueFromP3dFile;

      // get biodegradation constant from BioDegradIoTbl in project3d file
      bioDegMgr.getBioConstant(itr.first, valueFromP3dFile);
      ASSERT_NEAR(valueFromP3dFile, itr.second, 1e-6);

      valueFromP3dFile = 0.5555;
      // set biodegradation constant 0.5555
      bioDegMgr.setBioConstant(itr.first, valueFromP3dFile);
      // get biodegradation constant 0.5555
      bioDegMgr.getBioConstant(itr.first, valueFromP3dFile);
      ASSERT_NEAR(valueFromP3dFile, 0.5555, 1e-6);
   }
}

TEST_F(mbapiModelTest, BottomBoundaryManager)
{
   mbapi::Model testModel;
   mbapi::BottomBoundaryManager::BottomBoundaryModel model;
   mbapi::BottomBoundaryManager::CrustPropertyModel crustModel;
   mbapi::BottomBoundaryManager::MantlePropertyModel mantlModel;
   double mantleThickVal;

   // load test project
   ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile(m_fluidTestProject));

   mbapi::BottomBoundaryManager & botBoundMan = testModel.bottomBoundaryManager();

   //check whether bottom boundary models can be read and modified
   botBoundMan.getBottomBoundaryModel(model);
   EXPECT_EQ(mbapi::BottomBoundaryManager::BottomBoundaryModel::AdvancedCrustThinning, model);
   botBoundMan.setBottomBoundaryModel(mbapi::BottomBoundaryManager::BottomBoundaryModel::BaseSedimentHeatFlow);
   botBoundMan.getBottomBoundaryModel(model);
   EXPECT_EQ(mbapi::BottomBoundaryManager::BottomBoundaryModel::BaseSedimentHeatFlow, model);

   //check whether crust property models can be read and modified
   botBoundMan.getCrustPropertyModel(crustModel);
   EXPECT_EQ(mbapi::BottomBoundaryManager::CrustPropertyModel::LegacyCrust, crustModel);
   botBoundMan.setCrustPropertyModel(mbapi::BottomBoundaryManager::CrustPropertyModel::StandardCondModel);
   botBoundMan.getCrustPropertyModel(crustModel);
   EXPECT_EQ(mbapi::BottomBoundaryManager::CrustPropertyModel::StandardCondModel, crustModel);

   //check whether mantle property models can be read and modified
   botBoundMan.getMantlePropertyModel(mantlModel);
   EXPECT_EQ(mbapi::BottomBoundaryManager::MantlePropertyModel::LegacyMantle, mantlModel);
   botBoundMan.setMantlePropertyModel(mbapi::BottomBoundaryManager::MantlePropertyModel::HighCondMnModel);
   botBoundMan.getMantlePropertyModel(mantlModel);
   EXPECT_EQ(mbapi::BottomBoundaryManager::MantlePropertyModel::HighCondMnModel, mantlModel);

   //check whether initial lithospheric matle thickness can be read and modified correctly
   botBoundMan.getInitialLithoMantleThicknessValue(mantleThickVal);
   EXPECT_EQ(115000.0, mantleThickVal);
   double xyz = 125.0;
   botBoundMan.setInitialLithoMantleThicknessValue(xyz);
   botBoundMan.getInitialLithoMantleThicknessValue(mantleThickVal);
   EXPECT_EQ(xyz, mantleThickVal);

   //get time step ids
   auto timeStep = botBoundMan.getTimeStepsID();
   size_t actualTableSize = 2;
   //check whether all entries in CrustIOTbl were read
   ASSERT_EQ(actualTableSize, timeStep.size());

   //check whether Age and thickness can be read and modified from CrustIo table
   {
      mbapi::BottomBoundaryManager::TimeStepID id;
      std::vector<double> actualAges = { 0.0,100.0 };
      std::vector<double> actualThicknesses = { 30000.0,30000.0 };
      std::vector<std::string> actualThicknessesGrid = { "Crustal Thickness Map1", "Crustal Thickness Map2" };

      double age, thickness;
      std::string thicknessGrid;

      //Check whether all names are being read correctlty
      for (auto tsId : timeStep)
      {
         botBoundMan.getAge(tsId, age);
         EXPECT_NEAR(actualAges[tsId], age, eps);

         botBoundMan.getThickness(tsId, thickness);
         EXPECT_NEAR(actualThicknesses[tsId], thickness, eps);

         botBoundMan.getCrustThicknessGrid(tsId, thicknessGrid);
         EXPECT_EQ(actualThicknessesGrid[tsId], thicknessGrid);

      }

      id = 1;

      //check whether all ages of CrustIo table are being read and modified 
      botBoundMan.getAge(id, age);
      botBoundMan.setAge(id, 150.0);
      botBoundMan.getAge(id, age);
      EXPECT_NEAR(150.0, age, eps);
      botBoundMan.setAge(id, 0.0);
      botBoundMan.getAge(id, age);
      EXPECT_NEAR(0.0, age, eps);

      //check whether all thicknesses of CrustIo table are being read and modified 
      botBoundMan.getThickness(id, thickness);
      botBoundMan.setThickness(id, 30001.0);
      botBoundMan.getThickness(id, thickness);
      EXPECT_NEAR(30001.0, thickness, eps);
      botBoundMan.setThickness(id, 30000.0);
      botBoundMan.getThickness(id, thickness);
      EXPECT_NEAR(30000.0, thickness, eps);

      botBoundMan.getCrustThicknessGrid(id, thicknessGrid);
      botBoundMan.setCrustThicknessGrid(id, "Crustal Thickness Map3");
      botBoundMan.getCrustThicknessGrid(id, thicknessGrid);
      EXPECT_EQ("Crustal Thickness Map3", thicknessGrid);

   }

   //get time step ids
   timeStep = botBoundMan.getContCrustTimeStepsID();

   actualTableSize = 2;
   //check whether all entries in CrustIOTbl were read
   ASSERT_EQ(actualTableSize, timeStep.size());

   //check whether Age and thickness can be read and modified from ContCrustalThicknessIo table
   {
      mbapi::BottomBoundaryManager::ContCrustTimeStepID id1;
      std::vector<double> actualContCrustAges = { 0.0,100.0 };
      std::vector<double> actualContCrustThicknesses = { 35000.0,35000.0 };
      std::vector<std::string> actualContThicknessesGrid = { "MAP Name1", "MAP Name2" };

      double contAge, contThickness;
      std::string contThicknessGrid;

      //Check whether all names are being read correctlty
      for (auto tsId : timeStep)
      {
         botBoundMan.getContCrustAge(tsId, contAge);
         EXPECT_NEAR(actualContCrustAges[tsId], contAge, eps);

         botBoundMan.getContCrustThickness(tsId, contThickness);
         EXPECT_NEAR(actualContCrustThicknesses[tsId], contThickness, eps);

         botBoundMan.getContCrustThicknessGrid(tsId, contThicknessGrid);
         EXPECT_EQ(actualContThicknessesGrid[tsId], contThicknessGrid);
      }

      id1 = 0;

      //check whether all ages of ContCrustalThicknessIo table are being read and modified 
      botBoundMan.getContCrustAge(id1, contAge);
      botBoundMan.setContCrustAge(id1, 150.0);
      botBoundMan.getContCrustAge(id1, contAge);
      EXPECT_NEAR(150.0, contAge, eps);
      botBoundMan.setContCrustAge(id1, 0.0);
      botBoundMan.getContCrustAge(id1, contAge);
      EXPECT_NEAR(0.0, contAge, eps);

      //check whether all thicknessess of ContCrustalThicknessIo table are being read and modified
      botBoundMan.getContCrustThickness(id1, contThickness);
      botBoundMan.setContCrustThickness(id1, 35002.0);
      botBoundMan.getContCrustThickness(id1, contThickness);
      EXPECT_NEAR(35002.0, contThickness, eps);
      botBoundMan.setContCrustThickness(id1, 35000.0);
      botBoundMan.getContCrustThickness(id1, contThickness);
      EXPECT_NEAR(35000.0, contThickness, eps);

      botBoundMan.getContCrustThicknessGrid(id1, contThicknessGrid);
      botBoundMan.setContCrustThicknessGrid(id1, "MAP Name4");
      botBoundMan.getContCrustThicknessGrid(id1, contThicknessGrid);
      EXPECT_EQ("MAP Name4", contThicknessGrid);

      id1 = 1;

      //check whether all ages of ContCrustalThicknessIo table are being read and modified 
      botBoundMan.getContCrustAge(id1, contAge);
      botBoundMan.setContCrustAge(id1, 150.0);
      botBoundMan.getContCrustAge(id1, contAge);
      EXPECT_NEAR(150.0, contAge, eps);
      botBoundMan.setContCrustAge(id1, 100.0);
      botBoundMan.getContCrustAge(id1, contAge);
      EXPECT_NEAR(100.0, contAge, eps);

      //check whether all thicknessess of ContCrustalThicknessIo table are being read and modified
      botBoundMan.getContCrustThickness(id1, contThickness);
      botBoundMan.setContCrustThickness(id1, 35001.0);
      botBoundMan.getContCrustThickness(id1, contThickness);
      EXPECT_NEAR(35001.0, contThickness, eps);
      botBoundMan.setContCrustThickness(id1, 35000.0);
      botBoundMan.getContCrustThickness(id1, contThickness);
      EXPECT_NEAR(35000.0, contThickness, eps);

      botBoundMan.getContCrustThicknessGrid(id1, contThicknessGrid);
      botBoundMan.setContCrustThicknessGrid(id1, "MAP Name3");
      botBoundMan.getContCrustThicknessGrid(id1, contThicknessGrid);
      EXPECT_EQ("MAP Name3", contThicknessGrid);

   }
   //get time step ids
   timeStep = botBoundMan.getOceaCrustTimeStepsID();

   actualTableSize = 3;
   //check whether all entries in OceaCrustalThicknessIoTbl were read
   ASSERT_EQ(actualTableSize, timeStep.size());

   //check whether Age and thickness can be read and modified from OceaCrustalThicknessIoTbl
   {
      mbapi::BottomBoundaryManager::OceaCrustTimeStepID id2;
      std::vector<double> actualOceaAges = { 0.0,5.0,17.0 };
      std::vector<double> actualOceaThicknesses = { 0.0,150.0,320.0 };

      double oceaAge, oceaThickness;

      //Check whether all names are being read correctlty
      for (auto tsId : timeStep)
      {
         botBoundMan.getOceaCrustAge(tsId, oceaAge);
         EXPECT_NEAR(actualOceaAges[tsId], oceaAge, eps);

         botBoundMan.getOceaCrustThickness(tsId, oceaThickness);
         EXPECT_NEAR(actualOceaThicknesses[tsId], oceaThickness, eps);
      }

      id2 = 0;

      //check whether all ages of OceaCrustalThicknessIoTbl are being read and modified 
      botBoundMan.getOceaCrustAge(id2, oceaAge);
      botBoundMan.setOceaCrustAge(id2, 75.0);
      botBoundMan.getOceaCrustAge(id2, oceaAge);
      EXPECT_NEAR(75.0, oceaAge, eps);
      botBoundMan.setOceaCrustAge(id2, 0.0);
      botBoundMan.getOceaCrustAge(id2, oceaAge);
      EXPECT_NEAR(0.0, oceaAge, eps);

      //check whether all thicknessess of OceaCrustalThicknessIoTbl are being read and modified
      botBoundMan.getOceaCrustThickness(id2, oceaThickness);
      botBoundMan.setOceaCrustThickness(id2, 2.0);
      botBoundMan.getOceaCrustThickness(id2, oceaThickness);
      EXPECT_NEAR(2.0, oceaThickness, eps);
      botBoundMan.setOceaCrustThickness(id2, 0.0);
      botBoundMan.getOceaCrustThickness(id2, oceaThickness);
      EXPECT_NEAR(0.0, oceaThickness, eps);
   }


}

TEST_F(mbapiModelTest, CtcManager)
{
   mbapi::Model testModel;
   int filtrHalfWidth;
   double ULContCrustRat, ULOceaCrustRat;
   double age,depth,thickness;
   std::string TectonicContext,depthGrid,thicknessGrid;

   // load test project
   ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile(m_CtcTestProject));

   mbapi::CtcManager & ctcMan = testModel.ctcManager();
   mbapi::MapsManager & mapMan = testModel.mapsManager();

   //check whether filter half width value can be read and modified correctly
   ctcMan.getFilterHalfWidthValue(filtrHalfWidth);
   EXPECT_EQ(10, filtrHalfWidth);
   ctcMan.setFilterHalfWidthValue(6);
   ctcMan.getFilterHalfWidthValue(filtrHalfWidth);
   EXPECT_EQ(6, filtrHalfWidth);

   //check whether UpperLowerContinentalCrustRatio value can be read and modified correctly
   ctcMan.getUpperLowerContinentalCrustRatio(ULContCrustRat);
   EXPECT_EQ(1.0, ULContCrustRat);
   ctcMan.setUpperLowerContinentalCrustRatio(0.5);
   ctcMan.getUpperLowerContinentalCrustRatio(ULContCrustRat);
   EXPECT_EQ(0.5, ULContCrustRat);

   //check whether UpperLowerOceanicCrustRatio value can be read and modified correctly
   ctcMan.getUpperLowerOceanicCrustRatio(ULOceaCrustRat);
   EXPECT_EQ(1.0, ULOceaCrustRat);
   ctcMan.setUpperLowerOceanicCrustRatio(0.7);
   ctcMan.getUpperLowerOceanicCrustRatio(ULOceaCrustRat);
   EXPECT_EQ(0.7, ULOceaCrustRat);

   //get stratigraphic layer ids from StratIoTbl
   auto timeStep = ctcMan.getStratigraphyTblLayerID();
   size_t actualTableSize = 11;
   //check whether all entries in StratIOTbl were read
   ASSERT_EQ(actualTableSize, timeStep.size());

   //check whether Age can be read and modified from StratIo table
   {
      std::vector<double> actualDepositionalAges = { 0.0,5.0,13.0,54.0,65.0,90.0,142.0,146.0,150.0,155.0,300.0 };
      
      //Check whether all entries are being read correctlty
      for (auto tsId : timeStep)
      {
         ctcMan.getDepoAge(tsId, age);
         EXPECT_NEAR(actualDepositionalAges[tsId], age, eps);
      }
      mbapi::CtcManager::StratigraphyTblLayerID id;
      id = 1;
      //check whether the age of StratIo table are being read for id=2 
      ctcMan.getDepoAge(id, age);
      EXPECT_NEAR(5.0, age, eps);
      id = 6;
      //check whether the age of StratIo table are being read for id=6 
      ctcMan.getDepoAge(id, age);
      EXPECT_NEAR(142.0, age, eps);
   }
   //get time step ids from CTCRiftingHistoryIoTbl
   timeStep = ctcMan.getTimeStepID();
   actualTableSize = 3;
   //check whether all entries in CTCRiftingHistoryIoTbl were read
   ASSERT_EQ(actualTableSize, timeStep.size());
   //check whether Age can be read and modified from CTCRiftingHistoryIoTbl
   {
      std::vector<double> actualAges = { 10.0,20.0,30.0 };
      std::vector<std::string> actualTectonicContext = {"Active Rifting", "Passive Margin", "Flexural Basin"};
      std::vector<double> actualDepth = { -500.0,-501.0,-502.0 };
      std::vector<std::string> actualDepthMap = { "DeltaSlGridName1", "DeltaSlGridName2", "DeltaSlGridName3" };
      std::vector<double> actualBasaltMeltDepth = { 120.0,150.0,180.0 };
      std::vector<std::string> actualBasaltMeltMap = { "BasaltThicknessMap_1", "BasaltThicknessMap_2", "BasaltThicknessMap_3" };
      
      //Check whether all entries are being read correctlty
      for (auto tsId : timeStep)
      {
         ctcMan.getCTCRiftingHistoryTblAge(tsId, age);
         EXPECT_NEAR(actualAges[tsId], age, eps);

         ctcMan.getTectonicFlag(tsId, TectonicContext);
         EXPECT_EQ(actualTectonicContext[tsId], TectonicContext);

         ctcMan.getRiftingTblResidualDepthAnomalyScalar(tsId, depth);
         EXPECT_EQ(actualDepth[tsId],depth);

         ctcMan.getRiftingTblResidualDepthAnomalyMap(tsId, depthGrid);
         EXPECT_EQ(actualDepthMap[tsId], depthGrid);

         ctcMan.getRiftingTblBasaltMeltThicknessScalar(tsId, thickness);
         EXPECT_EQ(actualBasaltMeltDepth[tsId],thickness);

         ctcMan.getRiftingTblBasaltMeltThicknessMap(tsId,thicknessGrid);
         EXPECT_EQ(actualBasaltMeltMap[tsId], thicknessGrid);
         
      }
      mbapi::CtcManager::TimeStepID id1;
      id1 = 1;
      //check whether the age of CTCRiftingHistoryIoTbl are being read/modified for id=1 
      ctcMan.getCTCRiftingHistoryTblAge(id1, age);
      EXPECT_NEAR(20.0, age, eps);
      //check whether the tectonicFlag of CTCRiftingHistoryIoTbl are being read/modified for id=1 
      ctcMan.getTectonicFlag(id1, TectonicContext);
      EXPECT_EQ("Passive Margin", TectonicContext);
      ctcMan.setTectonicFlag(id1, "Flexural Basin");
      ctcMan.getTectonicFlag(id1, TectonicContext);
      EXPECT_EQ("Flexural Basin", TectonicContext);
      //check whether the relative sealevel adjustment value/map of CTCRiftingHistoryIoTbl are being read/modified for id=1 
      ctcMan.getRiftingTblResidualDepthAnomalyScalar(id1, depth);
      EXPECT_EQ(-501.0,depth);
      ctcMan.setRiftingTblResidualDepthAnomalyScalar(id1, 450.0);
      ctcMan.getRiftingTblResidualDepthAnomalyScalar(id1, depth);
      EXPECT_EQ(450.0, depth);
      ctcMan.getRiftingTblResidualDepthAnomalyMap(id1, depthGrid);
      EXPECT_EQ("DeltaSlGridName2", depthGrid);
      depthGrid = "DeltaSlGridName5";
      ctcMan.setRiftingTblResidualDepthAnomalyMap(id1, depthGrid);
      ctcMan.getRiftingTblResidualDepthAnomalyMap(id1, depthGrid);
      EXPECT_EQ("DeltaSlGridName5", depthGrid);
      //check whether the maximum thickness of basalt melt value/map of CTCRiftingHistoryIoTbl are being read/modified for id=1 
      ctcMan.getRiftingTblBasaltMeltThicknessScalar(id1, thickness);
      EXPECT_EQ(150.0, thickness);
      ctcMan.setRiftingTblBasaltMeltThicknessScalar(id1,175.0);
      ctcMan.getRiftingTblBasaltMeltThicknessScalar(id1, thickness);
      EXPECT_EQ(175.0, thickness);
      ctcMan.getRiftingTblBasaltMeltThicknessMap(id1,thicknessGrid);
      EXPECT_EQ("BasaltThicknessMap_2", thicknessGrid);
      ctcMan.setRiftingTblBasaltMeltThicknessMap(id1, "BasaltThicknessMap_2_new");
      ctcMan.getRiftingTblBasaltMeltThicknessMap(id1, thicknessGrid);
      EXPECT_EQ("BasaltThicknessMap_2_new", thicknessGrid);

      id1 = 2;
      //check whether the age of CTCRiftingHistoryIoTbl are being read/modified for id=2 
      ctcMan.getCTCRiftingHistoryTblAge(id1, age);
      EXPECT_NEAR(30.0, age, eps);
      ctcMan.getTectonicFlag(id1, TectonicContext);
      EXPECT_EQ("Flexural Basin", TectonicContext);
      ctcMan.setTectonicFlag(id1, "Passive Margin");
      ctcMan.getTectonicFlag(id1, TectonicContext);
      EXPECT_EQ("Passive Margin", TectonicContext);
   }

   std::string MapName;

   ctcMan.getEndRiftingAgeMap(MapName);
   EXPECT_EQ("TRIni@0", MapName);
   ctcMan.getEndRiftingAge(age);
   EXPECT_NEAR(120.0, age, eps);

   std::string mapName = "MAP-1514100163-4";
   mbapi::MapsManager::MapID id = mapMan.findID(mapName);
  
   double minV, maxV;
   mapMan.mapValuesRange(id, minV, maxV);
   
   EXPECT_NEAR(22.586912, minV, eps1);
   EXPECT_NEAR(2500.5203001, maxV, eps1);

   double value;
   
   //check whether Relative sealevel adjustment value/map of CTCIoTbl are being read/modified
   ctcMan.getResidualDepthAnomalyScalar(value);
   EXPECT_EQ(-350.0, value);
   ctcMan.setResidualDepthAnomalyScalar(-300.0);
   ctcMan.getResidualDepthAnomalyScalar(value);
   EXPECT_EQ(-300.0, value);

   ctcMan.getResidualDepthAnomalyMap(mapName);
   EXPECT_EQ("RDA_MapNAme", mapName);
   ctcMan.setResidualDepthAnomalyMap("RDA_Map_New");
   ctcMan.getResidualDepthAnomalyMap(mapName);
   EXPECT_EQ("RDA_Map_New", mapName);

   //check whether Maximum thickness of basalt melt value/map of CTCIoTbl are being read/modified
   ctcMan.getBasaltMeltThicknessValue(value);
   EXPECT_EQ(7000.0, value);
   ctcMan.setBasaltMeltThicknessValue(8000.0);
   ctcMan.getBasaltMeltThicknessValue(value);
   EXPECT_EQ(8000.0, value);

   ctcMan.getBasaltMeltThicknessMap(mapName);
   EXPECT_EQ("BasaltThicknessMap", mapName);
   ctcMan.setBasaltMeltThicknessMap("new_BasaltThicknessMap");
   ctcMan.getBasaltMeltThicknessMap(mapName);
   EXPECT_EQ("new_BasaltThicknessMap", mapName);

   //get row ids from GridMapIoTbl
   auto GridMapId = ctcMan.getGridMapID();
   actualTableSize = 10;
   //check whether number of records of GridMapIoTbl were read
   ASSERT_EQ(actualTableSize, GridMapId.size());

   //check whether table names can be read and modified from GridMapIo table
   {
      std::vector<std::string> actualTblNames = { "StratIoTbl","CTCIoTbl","StratIoTbl","StratIoTbl","StratIoTbl","CTCIoTbl","StratIoTbl","StratIoTbl","StratIoTbl","CTCIoTbl" };
      std::vector<std::string> actualMapNames = { "MAP-1514100159-4","MAP-1514100171-4","MAP-1514100169-4","MAP-1514100163-4","MAP-1514100167-4","MAP-1514100157-4","MAP-1514100173-4","MAP-1514100161-4","MAP-1514100165-4","MAP-1514100155-4" };

      //Check whether all entries are being read correctlty
      for (auto tsId : GridMapId)
      {
         ctcMan.getGridMapTablename(tsId, mapName);
         EXPECT_EQ(actualTblNames[tsId], mapName);

         ctcMan.getGridMapIoTblMapName(tsId,thicknessGrid);
         EXPECT_EQ(actualMapNames[tsId], thicknessGrid);
      }
      mbapi::CtcManager::GridMapID id2;
      id2 = 1;
      //check whether the ReferredBy and MapName fields are being read/modified for id2=1
      ctcMan.getGridMapTablename(id2, mapName);
      EXPECT_EQ("CTCIoTbl", mapName);
      ctcMan.setGridMapTablename(id2,"SourceRockLithIoTbl");
      ctcMan.getGridMapTablename(id2, mapName);
      EXPECT_EQ("SourceRockLithIoTbl", mapName);

      ctcMan.getGridMapIoTblMapName(id2,thicknessGrid);
      EXPECT_EQ("MAP-1514100171-4", thicknessGrid);
      ctcMan.setGridMapIoTblMapName(id2, "Expedite_Map_Name");
      ctcMan.getGridMapIoTblMapName(id2, thicknessGrid);
      EXPECT_EQ("Expedite_Map_Name", thicknessGrid);
      
   }

}
