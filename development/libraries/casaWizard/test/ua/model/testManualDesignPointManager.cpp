#include "model/manualDesignPointManager.h"

#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include "expectFileEq.h"

#include <gtest/gtest.h>

TEST( ManualDesignPointManagerTest, addDesignPoint )
{
  casaWizard::ua::ManualDesignPointManager manager;
  manager.addDesignPoint();
  ASSERT_EQ(1, manager.numberOfPoints());
}

TEST( ManualDesignPointManagerTest, addInfluentialParameter )
{
  casaWizard::ua::ManualDesignPointManager manager;
  manager.addInfluentialParameter();
  ASSERT_EQ(1, manager.numberOfParameters());
}

TEST( ManualDesignPointManagerTest, addInfluentialParameters )
{
  casaWizard::ua::ManualDesignPointManager manager;
  manager.addInfluentialParameter();
  manager.addInfluentialParameter();
  ASSERT_EQ(2, manager.numberOfParameters());
}

TEST( ManualDesignPointManagerTest, addMultipleInfluentialParameters )
{
  casaWizard::ua::ManualDesignPointManager manager;
  manager.addInfluentialParameter(4);
  ASSERT_EQ(4, manager.numberOfParameters());
}

TEST( ManualDesignPointManagerTest, addParameterFirst)
{
  casaWizard::ua::ManualDesignPointManager manager;
  manager.addInfluentialParameter(2);
  manager.addDesignPoint();
  QVector<double> values = manager.getDesignPoint(0);
  ASSERT_EQ(2, values.size());
  EXPECT_EQ(0, values[0]);
  EXPECT_EQ(0, values[1]);
}

TEST( ManualDesignPointManagerTest, addDesignPointFirst)
{
  casaWizard::ua::ManualDesignPointManager manager;
  manager.addDesignPoint();
  manager.addInfluentialParameter(2);
  QVector<double> values = manager.getDesignPoint(0);
  ASSERT_EQ(2, values.size());
  EXPECT_EQ(0, values[0]);
  EXPECT_EQ(0, values[1]);
}

TEST( ManualDesignPointManagerTest, setParameterValues)
{
  casaWizard::ua::ManualDesignPointManager manager;
  manager.addDesignPoint();
  manager.addInfluentialParameter(2);
  manager.setParameterValue(0, 0, 10);
  manager.setParameterValue(0, 1, 11);
  QVector<double> values = manager.getDesignPoint(0);
  ASSERT_EQ(2, values.size());
  EXPECT_EQ(10, values[0]);
  EXPECT_EQ(11, values[1]);
}

TEST( ManualDesignPointManagerTest, addDesignPointByVector )
{
  casaWizard::ua::ManualDesignPointManager manager;
  manager.addInfluentialParameter(3);
  QVector<double> newDesignPoint{11,12,13};
  manager.addDesignPoint(newDesignPoint);

  ASSERT_EQ(3, manager.numberOfParameters());
  ASSERT_EQ(1, manager.numberOfPoints());

  QVector<double> designPoint = manager.getDesignPoint(0);
  for (int i = 0; i < 3; ++i)
  {
    EXPECT_EQ( newDesignPoint[i], designPoint[i]) << " Mismatch at index [" << i << "]";
  }
}

TEST( ManualDesignPointManagerTest, removeParameter )
{
  casaWizard::ua::ManualDesignPointManager manager;
  manager.addInfluentialParameter(3);
  QVector<double> newDesignPoint{11,12,13};
  manager.addDesignPoint(newDesignPoint);

  manager.removeInfluentialParameter(1);

  QVector<double> designPoint = manager.getDesignPoint(0);

  ASSERT_EQ(2, designPoint.size());
  EXPECT_EQ(11, designPoint[0]);
  EXPECT_EQ(13, designPoint[1]);
}

TEST( ManualDesignPointManagerTest, removeDesignPoint )
{
  casaWizard::ua::ManualDesignPointManager manager;
  manager.addInfluentialParameter(2);
  manager.addDesignPoint({11,12});
  manager.removeDesignPoint(0);
  ASSERT_EQ(0, manager.numberOfPoints());
}

TEST( ManualDesignPointManagerTest, removeCompletedDesignPoint )
{
  casaWizard::ua::ManualDesignPointManager manager;
  manager.addInfluentialParameter(2);
  manager.addDesignPoint({11,12});
  manager.completeAll();
  manager.removeDesignPoint(0);
  ASSERT_EQ(1, manager.numberOfPoints());
}

TEST( ManualDesignPointManagerTest, modifyCase )
{
  casaWizard::ua::ManualDesignPointManager manager;
  manager.addInfluentialParameter();
  manager.addDesignPoint({10});
  manager.setParameterValue(0, 0, 20);
  ASSERT_EQ(20, manager.getDesignPoint(0)[0]);
}

TEST( ManualDesignPointManagerTest, modifyCompletedCase )
{
  casaWizard::ua::ManualDesignPointManager manager;
  manager.addInfluentialParameter();
  manager.addDesignPoint({10});
  manager.completeAll();
  manager.setParameterValue(0, 0, 20);
  ASSERT_EQ(10, manager.getDesignPoint(0)[0]);
}

TEST( ManualDesignPointManagerTest, getNumberOfCasesToRun )
{
  casaWizard::ua::ManualDesignPointManager manager;
  manager.addInfluentialParameter(2);
  manager.addDesignPoint({1,2});
  manager.addDesignPoint({3,4});

  EXPECT_EQ(2, manager.numberOfCasesToRun());
  manager.completeAll();

  EXPECT_EQ(0, manager.numberOfCasesToRun());
}

TEST( ManualDesignPointManagerTest, writeToFile )
{
  casaWizard::ua::ManualDesignPointManager manager;
  manager.addInfluentialParameter(2);
  manager.addDesignPoint({11, 12});
  manager.completeAll();
  manager.addDesignPoint({21, 22});

  casaWizard::ScenarioWriter writer{"manualDesignPointManagerActual.dat"};
  manager.writeToFile(writer);
  writer.close();

  expectFileEq("manualDesignPointManager.dat", "manualDesignPointManagerActual.dat");
}

TEST( ManualDesignPointManagerTest, readFromFile )
{
  casaWizard::ua::ManualDesignPointManager manager;
  casaWizard::ScenarioReader reader{"manualDesignPointManager.dat"};
  manager.readFromFile(reader);

  ASSERT_EQ(2, manager.numberOfParameters());
  ASSERT_EQ(2, manager.numberOfPoints());

  QVector<double> designPoint0 = manager.getDesignPoint(0);
  QVector<double> designPoint1 = manager.getDesignPoint(1);

  EXPECT_EQ(11, designPoint0[0]);
  EXPECT_EQ(12, designPoint0[1]);
  EXPECT_EQ(21, designPoint1[0]);
  EXPECT_EQ(22, designPoint1[1]);

  QVector<bool> completed = manager.completed();
  EXPECT_TRUE(completed[0]);
  EXPECT_FALSE(completed[1]);
}
