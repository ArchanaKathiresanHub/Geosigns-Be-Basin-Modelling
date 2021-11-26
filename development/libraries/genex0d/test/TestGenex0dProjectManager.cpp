#include "Genex0dProjectManager.h"

#include "ObjectFactory.h"
#include "ProjectHandle.h"

#include <gtest/gtest.h>

namespace
{

const double epsilon = 1e-6;
const std::string formationName = "Formation6";
const std::string topSurfaceName = "Surface5";
const double xCoord = 2005.0;
const double yCoord = 8456.0;

} // namespace

class TestGenex0dProjectManager : public ::testing::Test
{
protected:
  void SetUp() final
  {
    objectFactory = new DataAccess::Interface::ObjectFactory();
    projectHandle = DataAccess::Interface::OpenCauldronProject(projectFileName, objectFactory);
    ASSERT_NO_THROW(genex0dProjectMgr =
        new Genex0d::Genex0dProjectManager(*projectHandle, projectFileName, xCoord, yCoord, topSurfaceName, formationName));
  }

  void TearDown() final
  {
    if (genex0dProjectMgr) delete genex0dProjectMgr;
    if (projectHandle) delete projectHandle;
    if (objectFactory) delete objectFactory;
  }

  static const std::string projectFileName;
  DataAccess::Interface::ObjectFactory * objectFactory;
  DataAccess::Interface::ProjectHandle * projectHandle;
  Genex0d::Genex0dProjectManager * genex0dProjectMgr;
};
const std::string TestGenex0dProjectManager::projectFileName = "AcquiferScale1.project3d";

TEST_F( TestGenex0dProjectManager, TestcomputeAgesFromAllSnapShots )
{
  std::vector<double> agesExpected = {33.0000000, 31.9111560, 30.8823865, 29.8724824, 28.8594945, 27.8432401, 27.0000000, 26.2500000, 25.6225826, 24.9021500, 24.0806289,
                                      23.4705701, 23.0000000, 21.3125000, 19.9394985, 17.8874298, 16.4601443, 15.0591761, 14.0000000, 12.8762214, 11.8849351, 10.8657666,
                                      9.8499416, 8.8384686, 8.0000000, 7.5000000, 7.2500000, 6.9842912, 6.5229775, 6.2638751, 6.0000000, 5.5000000, 5.2277559, 4.9747352,
                                      4.5001344, 4.2399230, 4.0000000, 3.5000000, 3.2500000, 2.9865397, 2.4868929, 2.2486188, 2.0000000, 1.5000000, 1.2357986, 0.9933745,
                                      0.5157488, 0.2655320, 0.0};

  const double depoAge = 33.0;
  EXPECT_NO_THROW(genex0dProjectMgr->computeAgesFromAllSnapShots(depoAge));
  std::vector<double> agesActual = genex0dProjectMgr->agesAll();

  int i = 0;
  for (auto tExpected : agesExpected)
  {
    EXPECT_NEAR(tExpected, agesActual[i], epsilon);
    ++i;
  }
}

TEST_F( TestGenex0dProjectManager, TestRequestPropertyHistory )
{
  const double depoAge = 33.0;
  EXPECT_NO_THROW(genex0dProjectMgr->computeAgesFromAllSnapShots(depoAge));

  std::string propertyName = "Temperature";
  EXPECT_NO_THROW(genex0dProjectMgr->requestPropertyHistory(propertyName));

  propertyName = "Ves";
  EXPECT_NO_THROW(genex0dProjectMgr->requestPropertyHistory(propertyName));
}

TEST_F( TestGenex0dProjectManager, TestGetValues )
{
  const double depoAge = 33.0;
  EXPECT_NO_THROW(genex0dProjectMgr->computeAgesFromAllSnapShots(depoAge));

  std::string propertyName = "Temperature";
  std::vector<double> temperaturesActual;
  EXPECT_NO_THROW(genex0dProjectMgr->requestPropertyHistory(propertyName));

  propertyName = "Ves";
  std::vector<double> vesActual;
  EXPECT_NO_THROW(genex0dProjectMgr->requestPropertyHistory(propertyName));

  EXPECT_NO_THROW(genex0dProjectMgr->extract());
  EXPECT_NO_THROW(temperaturesActual = genex0dProjectMgr->getValues("Temperature"));
  EXPECT_NO_THROW(vesActual = genex0dProjectMgr->getValues("Ves"));

  std::vector<double> temperaturesExpected = {10.0000000, 13.9797779, 17.5144900, 20.8070680, 23.9542304, 26.9708525, 29.3703104, 35.2830657, 39.7960663,
                                              44.5171140, 49.3919296, 52.7093863, 55.0993286, 56.7373903, 58.1803674, 61.0572944, 63.3591036, 65.7347682,
                                              67.5628889, 77.0094396, 84.4506024, 91.0031300, 96.7409886, 101.8835254, 105.8059608, 108.0662195, 109.0960273,
                                              110.1890048, 112.0632678, 113.1119870, 114.1571958, 116.1791855, 117.2182102, 118.1772734, 119.9552955, 120.9234220,
                                              121.8020922, 123.6694278, 124.5505543, 125.4763937, 127.2190061, 128.0419582, 128.8964160, 130.6461445, 131.5233673,
                                              132.3232935, 133.8957759, 134.7169255};

  std::vector<double> vesExpected = {0.0000000, 896401.4051400, 1743345.8233500, 2574759.2782400, 3408711.2348800, 4245352.7014400, 4939558.2775600, 5501218.6667600,
                                     5971079.6012800, 6510598.1370400, 7125819.6168800, 7582681.1951600, 7935081.9085200, 9715854.0417600, 11164744.0555200, 13330234.3705600,
                                     14836409.4716000, 16314812.1704000, 17432530.4937600, 19267790.7468800, 20886674.5657600, 22551093.9515200, 24210051.0872000, 25861902.5001600,
                                     27231219.0400000, 27990197.0400000, 28369686.8457600, 28773020.8457600, 29473275.0400000, 29866581.0400000, 30267131.0400000, 31026110.8457600,
                                     31439365.0400000, 31823439.0400000, 32543863.0400000, 32938853.0382400, 33303045.0400000, 34062023.0400000, 34441511.0417600, 34841435.0400000,
                                     35599875.0400000, 35961567.0400000, 36338959.0400000, 37097935.0400000, 37498983.0400000, 37866971.0400000, 38591987.0400000, 38971807.0400000};

  int i = 0;
  for (auto tempExpected : temperaturesExpected)
  {
    EXPECT_NEAR(tempExpected, temperaturesActual[i], epsilon);
    ++i;
  }

  i = 0;
  for (auto pExpected : vesExpected)
  {
    EXPECT_NEAR(pExpected, vesActual[i], epsilon);
    ++i;
  }
}
