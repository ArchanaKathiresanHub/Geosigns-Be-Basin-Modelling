#include "../src/RunCaseSetImpl.h"

#include "../src/RunCaseImpl.h"
#include "../src/Parameter.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

class MockParameter : public casa::Parameter
{
public:
  MockParameter(const std::vector<double>& values) : values_(values) {}
  MOCK_CONST_METHOD0(parent, casa::VarParameter*());
  MOCK_METHOD1(setParent, void(const casa::VarParameter*));
  MOCK_CONST_METHOD0(appSolverDependencyLevel, casa::AppPipelineLevel());
  MOCK_METHOD2(setInModel, ErrorHandler::ReturnCode( mbapi::Model&, size_t));
  MOCK_METHOD1(validate, std::string(mbapi::Model&));
  MOCK_CONST_METHOD0(asInteger, int());
  MOCK_CONST_METHOD1(save, bool(casa::CasaSerializer&));
  MOCK_CONST_METHOD0(typeName, const char*());
  bool operator==(const casa::Parameter& prm) const
  {
    return values_ == prm.asDoubleArray();
  }
  std::vector<double> asDoubleArray() const { return values_; }
private:
  std::vector<double> values_;
};

class RunCaseSetTest : public ::testing::Test
{
public:
  void SetUp() override
  {
    runCase1->addParameter(parameter1);
    runCase2->addParameter(parameter2);
    runCase3->addParameter(parameter3);
    runCase4->addParameter(parameter4);
    runCase5->addParameter(parameter5);
  }

  SharedParameterPtr parameter1{new MockParameter({1.0})};
  SharedParameterPtr parameter2{new MockParameter({2.0})};
  SharedParameterPtr parameter3{new MockParameter({3.0})};
  SharedParameterPtr parameter4{new MockParameter({4.0})};
  SharedParameterPtr parameter5{new MockParameter({5.0})};

  std::shared_ptr<casa::RunCase> runCase1{new casa::RunCaseImpl()};
  std::shared_ptr<casa::RunCase> runCase2{new casa::RunCaseImpl()};
  std::shared_ptr<casa::RunCase> runCase3{new casa::RunCaseImpl()};
  std::shared_ptr<casa::RunCase> runCase4{new casa::RunCaseImpl()};
  std::shared_ptr<casa::RunCase> runCase5{new casa::RunCaseImpl()};
};

TEST_F(RunCaseSetTest, TestAddNewCases)
{
  casa::RunCaseSetImpl runCaseSet;

  std::vector<std::shared_ptr<casa::RunCase>> runCases;
  std::shared_ptr<casa::RunCase> runCase(new casa::RunCaseImpl());
  runCases.push_back(runCase);
  runCases.push_back(runCase);
  runCaseSet.addNewCases(runCases, "LabelA");

  ASSERT_EQ(2, runCaseSet.size());
}

TEST_F(RunCaseSetTest, TestDuplicateRunCase)
{
  casa::RunCaseSetImpl runCaseSet;

  std::vector<std::shared_ptr<casa::RunCase>> runCasesA;

  runCasesA.push_back(runCase1);
  runCasesA.push_back(runCase2);

  runCaseSet.addNewCases(runCasesA, "LabelA");

  std::vector<std::shared_ptr<casa::RunCase>> runCasesB;
  runCasesB.push_back(runCase1);
  runCasesB.push_back(runCase2);
  runCasesB.push_back(runCase3);
  runCaseSet.addNewCases(runCasesB, "LabelB");

  ASSERT_EQ(3, runCaseSet.size());
}

TEST_F(RunCaseSetTest, TestFilterByExperimentName)
{
  casa::RunCaseSetImpl runCaseSet;

  std::vector<std::shared_ptr<casa::RunCase>> runCasesA;
  std::vector<std::shared_ptr<casa::RunCase>> runCasesB;

  runCasesA.push_back(runCase1);
  runCasesA.push_back(runCase2);
  runCaseSet.addNewCases(runCasesA, "LabelA");

  runCasesB.push_back(runCase1);
  runCasesB.push_back(runCase3);
  runCaseSet.addNewCases(runCasesB, "LabelB");

  runCaseSet.filterByExperimentName("LabelA");
  EXPECT_EQ(2, runCaseSet.size());

  EXPECT_EQ(1.0, runCaseSet[0]->parameter(0)->asDoubleArray()[0]);
  EXPECT_EQ(2.0, runCaseSet[1]->parameter(0)->asDoubleArray()[0]);

  runCaseSet.filterByExperimentName("LabelB");
  EXPECT_EQ(2, runCaseSet.size());

  EXPECT_EQ(1.0, runCaseSet[0]->parameter(0)->asDoubleArray()[0]);
  EXPECT_EQ(3.0, runCaseSet[1]->parameter(0)->asDoubleArray()[0]);
}


TEST_F(RunCaseSetTest, TestFilterByDoeListWithoutOverlap)
{
  casa::RunCaseSetImpl runCaseSet;

  std::vector<std::shared_ptr<casa::RunCase>> runCasesA;
  std::vector<std::shared_ptr<casa::RunCase>> runCasesB;
  std::vector<std::shared_ptr<casa::RunCase>> runCasesC;

  runCasesA.push_back(runCase1);
  runCasesA.push_back(runCase2);
  runCasesB.push_back(runCase3);
  runCasesC.push_back(runCase4);
  runCasesC.push_back(runCase5);

  runCaseSet.addNewCases(runCasesA, "LabelA");
  runCaseSet.addNewCases(runCasesB, "LabelB");
  runCaseSet.addNewCases(runCasesC, "LabelC");

  runCaseSet.filterByDoeList({"LabelA", "LabelC"});
  EXPECT_EQ(4, runCaseSet.size());

  EXPECT_EQ(1.0, runCaseSet[0]->parameter(0)->asDoubleArray()[0]);
  EXPECT_EQ(2.0, runCaseSet[1]->parameter(0)->asDoubleArray()[0]);
  EXPECT_EQ(4.0, runCaseSet[2]->parameter(0)->asDoubleArray()[0]);
  EXPECT_EQ(5.0, runCaseSet[3]->parameter(0)->asDoubleArray()[0]);
}


TEST_F(RunCaseSetTest, TestFilterByDoeListWithOverlap)
{
  casa::RunCaseSetImpl runCaseSet;

  std::vector<std::shared_ptr<casa::RunCase>> runCasesA;
  std::vector<std::shared_ptr<casa::RunCase>> runCasesB;
  std::vector<std::shared_ptr<casa::RunCase>> runCasesC;

  runCasesA.push_back(runCase1);
  runCasesA.push_back(runCase2);
  runCasesA.push_back(runCase4);
  runCasesB.push_back(runCase3);
  runCasesC.push_back(runCase1);
  runCasesC.push_back(runCase4);
  runCasesC.push_back(runCase5);

  runCaseSet.addNewCases(runCasesA, "LabelA");
  runCaseSet.addNewCases(runCasesB, "LabelB");
  runCaseSet.addNewCases(runCasesC, "LabelC");

  runCaseSet.filterByDoeList({"LabelA", "LabelC"});
  EXPECT_EQ(4, runCaseSet.size());

  EXPECT_EQ(1.0, runCaseSet[0]->parameter(0)->asDoubleArray()[0]);
  EXPECT_EQ(2.0, runCaseSet[1]->parameter(0)->asDoubleArray()[0]);
  EXPECT_EQ(4.0, runCaseSet[2]->parameter(0)->asDoubleArray()[0]);
  EXPECT_EQ(5.0, runCaseSet[3]->parameter(0)->asDoubleArray()[0]);
}

TEST_F(RunCaseSetTest, TestAddSingleUserDefinedCase)
{
  casa::RunCaseSetImpl runCaseSet;
  runCaseSet.addUserDefinedCase(runCase1);
  runCaseSet.filterByDoeList({"UserDefined"});

  ASSERT_EQ(1, runCaseSet.size());
  EXPECT_EQ(1.0, runCaseSet[0]->parameter(0)->asDoubleArray()[0]);
}

TEST_F(RunCaseSetTest, TestAddTwoUserDefinedCases)
{
  casa::RunCaseSetImpl runCaseSet;
  runCaseSet.addUserDefinedCase(runCase1);
  runCaseSet.addUserDefinedCase(runCase2);
  runCaseSet.filterByDoeList({"UserDefined"});

  ASSERT_EQ(2, runCaseSet.size());
  EXPECT_EQ(1.0, runCaseSet[0]->parameter(0)->asDoubleArray()[0]);
  EXPECT_EQ(2.0, runCaseSet[1]->parameter(0)->asDoubleArray()[0]);
}

TEST_F(RunCaseSetTest, TestSelectUserDefinedFromLargerSet)
{
  casa::RunCaseSetImpl runCaseSet;

  std::vector<std::shared_ptr<casa::RunCase>> runCasesA;
  runCasesA.push_back(runCase3);
  runCasesA.push_back(runCase4);
  runCasesA.push_back(runCase5);
  runCaseSet.addNewCases(runCasesA, "DummyCases");

  runCaseSet.addUserDefinedCase(runCase1);
  runCaseSet.addUserDefinedCase(runCase2);
  runCaseSet.filterByDoeList({"UserDefined"});

  ASSERT_EQ(2, runCaseSet.size());
  EXPECT_EQ(1.0, runCaseSet[0]->parameter(0)->asDoubleArray()[0]);
  EXPECT_EQ(2.0, runCaseSet[1]->parameter(0)->asDoubleArray()[0]);
}

