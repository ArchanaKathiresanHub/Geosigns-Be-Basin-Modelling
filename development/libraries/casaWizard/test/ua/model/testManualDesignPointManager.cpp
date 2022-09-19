#include "model/manualDesignPointManager.h"

#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include "expectFileEq.h"

#include <QDir>

#include <gtest/gtest.h>

TEST( ManualDesignPointManagerTest, addDesignPoint )
{
   casaWizard::ua::ManualDesignPointManager manager;
   manager.addDesignPoint();
   ASSERT_EQ(1, manager.numberOfVisiblePoints());
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
   QVector<QVector<double>> values = manager.pointsToRun();
   ASSERT_EQ(2, values[0].size());
   EXPECT_EQ(0, values[0][0]);
   EXPECT_EQ(0, values[0][1]);
}

TEST( ManualDesignPointManagerTest, addDesignPointFirst)
{
   casaWizard::ua::ManualDesignPointManager manager;
   manager.addDesignPoint();
   manager.addInfluentialParameter(2);
   QVector<QVector<double>> values = manager.pointsToRun();
   ASSERT_EQ(2, values[0].size());
   EXPECT_EQ(0, values[0][0]);
   EXPECT_EQ(0, values[0][1]);
}

TEST( ManualDesignPointManagerTest, setParameterValues)
{
   casaWizard::ua::ManualDesignPointManager manager;
   manager.addDesignPoint();
   manager.addInfluentialParameter(2);
   manager.setParameterValue(0, 0, 10);
   manager.setParameterValue(0, 1, 11);
   QVector<QVector<double>> values = manager.pointsToRun();
   ASSERT_EQ(2, values[0].size());
   EXPECT_EQ(10, values[0][0]);
   EXPECT_EQ(11, values[0][1]);
}

TEST( ManualDesignPointManagerTest, addDesignPointByVector )
{
   casaWizard::ua::ManualDesignPointManager manager;
   manager.addInfluentialParameter(3);
   QVector<double> newDesignPoint{11,12,13};
   manager.addDesignPoint(newDesignPoint);

   QVector<double> newDesignPointTooClose{11,12,13.001};
   EXPECT_FALSE(manager.addDesignPoint(newDesignPointTooClose));
   EXPECT_EQ(manager.numberOfVisiblePoints(),1);

   ASSERT_EQ(3, manager.numberOfParameters());
   ASSERT_EQ(1, manager.numberOfVisiblePoints());

   QVector<QVector<double>> designPoint = manager.pointsToRun();
   for (int i = 0; i < 3; ++i)
   {
      EXPECT_EQ( newDesignPoint[i], designPoint[0][i]) << " Mismatch at index [" << i << "]";
   }
}

TEST( ManualDesignPointManagerTest, removeParameter )
{
   casaWizard::ua::ManualDesignPointManager manager;
   manager.addInfluentialParameter(3);
   QVector<double> newDesignPoint{11,12,13};
   manager.addDesignPoint(newDesignPoint);

   manager.removeInfluentialParameter(1);

   //Removing with invalid index does nothing:
   manager.removeInfluentialParameter(-1);
   manager.removeInfluentialParameter(2);

   QVector<QVector<double>> designPoints = manager.pointsToRun();

   ASSERT_EQ(2, designPoints[0].size());
   EXPECT_EQ(11, designPoints[0][0]);
   EXPECT_EQ(13, designPoints[0][1]);
}

TEST( ManualDesignPointManagerTest, removeDesignPoint )
{
   casaWizard::ua::ManualDesignPointManager manager;
   manager.addInfluentialParameter(2);
   manager.addDesignPoint({11,12});
   manager.removeDesignPoint(0);
   ASSERT_EQ(0, manager.numberOfVisiblePoints());
}

TEST( ManualDesignPointManagerTest, removeCompletedDesignPoint )
{
   casaWizard::ua::ManualDesignPointManager manager;
   manager.addInfluentialParameter(2);
   manager.addDesignPoint({11,12});
   manager.setCompleted(0);
   manager.removeDesignPoint(0);
   ASSERT_EQ(1, manager.numberOfVisiblePoints());
}

TEST( ManualDesignPointManagerTest, removeFailedDesignPoint )
{
   casaWizard::ua::ManualDesignPointManager manager;
   manager.addDesignPoint();
   manager.setCompleted(0,false);
   manager.removeDesignPoint(0);

   //Point is still there after removal, to maintain compatibility with the casa state.
   //However, it is hidden, and can no longer be retrieved from the manualDesignPointManager.
   ASSERT_EQ(0, manager.numberOfVisiblePoints());
   ASSERT_EQ(0, manager.parameters().size());
   ASSERT_EQ(0, manager.completed().size());
}

TEST( ManualDesignPointManagerTest, readAndSetCompletionStates )
{
   casaWizard::ua::ManualDesignPointManager manager;
   manager.addInfluentialParameter(1);
   manager.addDesignPoint({1});
   manager.addDesignPoint({2});

   QDir tmpTestDir(QDir::currentPath() + "/tmpTestDir");
   tmpTestDir.removeRecursively();
   tmpTestDir.mkdir(QDir::currentPath() + "/tmpTestDir");

   { //State file:
      QFile file{QDir::currentPath() + "/tmpTestDir/state"};
      file.open(QIODevice::WriteOnly | QIODevice::Text);
      QTextStream out(&file);
      out << "1\n0\n1";
      file.close();
   }

   { //doe indices file:
      QFile file{QDir::currentPath() + "/tmpTestDir/doeIndices"};
      file.open(QIODevice::WriteOnly | QIODevice::Text);
      QTextStream out(&file);
      out << "0\n1 2";
      file.close();
   }

   QStringList namesDoeOptionsSelected{"Tornado","UserDefined"};
   manager.readAndSetCompletionStates(QDir::currentPath() + "/tmpTestDir/state", QDir::currentPath() + "/tmpTestDir/doeIndices", namesDoeOptionsSelected);

   //The design point for which the simulation has failed is hidden and re-added as not run, such that the user can choose to rerun it.
   QVector<bool> completed = manager.completed();
   QVector<bool> expectedCompletionState{true,false};
   EXPECT_EQ(completed,expectedCompletionState);

   QVector<QVector<double>> parameters = manager.parameters();
   QVector<QVector<double>> expectedParameters{{2},{1}};
   EXPECT_EQ(parameters,expectedParameters);

   //Invalid input cases, should not crash:

   //MisMatch between doeNames and doe indices:
   manager.readAndSetCompletionStates(QDir::currentPath() + "/tmpTestDir/state", QDir::currentPath() + "/tmpTestDir/doeIndices", {"Tornado","UserDefined","thirDesign"});

   //Mismatch between number of manual design points and casa output:
   manager.addDesignPoint({3});
   manager.readAndSetCompletionStates(QDir::currentPath() + "/tmpTestDir/state", QDir::currentPath() + "/tmpTestDir/doeIndices", namesDoeOptionsSelected);

   //Mismatch between doeIndices and state file:
   { //doe indices file:
      QFile file{QDir::currentPath() + "/tmpTestDir/doeIndices"};
      file.open(QIODevice::WriteOnly | QIODevice::Text);
      QTextStream out(&file);
      out << "0\n1 2 3";
      file.close();
   }
   manager.readAndSetCompletionStates(QDir::currentPath() + "/tmpTestDir/state", QDir::currentPath() + "/tmpTestDir/doeIndices", namesDoeOptionsSelected);


   //cleanup
   tmpTestDir.removeRecursively();
}

TEST( ManualDesignPointManagerTest, modifyCase )
{
   casaWizard::ua::ManualDesignPointManager manager;
   manager.addInfluentialParameter();
   manager.addDesignPoint({10});
   manager.setParameterValue(0, 0, 20);
   ASSERT_EQ(20, manager.pointsToRun()[0][0]);
}

TEST( ManualDesignPointManagerTest, modifyCompletedCase )
{
   casaWizard::ua::ManualDesignPointManager manager;
   manager.addInfluentialParameter();
   manager.addDesignPoint({10});
   manager.setCompleted(0);
   manager.setParameterValue(0, 0, 20);
   ASSERT_EQ(10, manager.parameters()[0][0]);
}

TEST( ManualDesignPointManagerTest, getNumberOfCasesToRun )
{
   casaWizard::ua::ManualDesignPointManager manager;
   manager.addInfluentialParameter(2);
   manager.addDesignPoint({1,2});
   manager.addDesignPoint({3,4});

   EXPECT_EQ(2, manager.numberOfCasesToRun());
   manager.setCompleted(0);
   manager.setCompleted(1);

   EXPECT_EQ(0, manager.numberOfCasesToRun());
}

TEST( ManualDesignPointManagerTest, writeToFile )
{
   casaWizard::ua::ManualDesignPointManager manager;
   manager.addInfluentialParameter(2);
   manager.addDesignPoint({11, 12});
   manager.setCompleted(0);
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
   ASSERT_EQ(2, manager.numberOfVisiblePoints());

   QVector<QVector<double>> pointsToRun = manager.parameters();
   QVector<double> designPoint0 = pointsToRun[0];
   QVector<double> designPoint1 = pointsToRun[1];

   EXPECT_EQ(11, designPoint0[0]);
   EXPECT_EQ(12, designPoint0[1]);
   EXPECT_EQ(21, designPoint1[0]);
   EXPECT_EQ(22, designPoint1[1]);

   QVector<bool> completed = manager.completed();
   EXPECT_TRUE(completed[0]);
   EXPECT_FALSE(completed[1]);
}

TEST( ManualDesignPointManagerTest, readFromFileV0 )
{
   casaWizard::ua::ManualDesignPointManager manager;
   casaWizard::ScenarioReader reader{"manualDesignPointManagerV0.dat"};
   manager.readFromFile(reader);

   ASSERT_EQ(2, manager.numberOfParameters());
   ASSERT_EQ(2, manager.numberOfVisiblePoints());

   QVector<QVector<double>> pointsToRun = manager.parameters();
   QVector<double> designPoint0 = pointsToRun[0];
   QVector<double> designPoint1 = pointsToRun[1];

   EXPECT_EQ(11, designPoint0[0]);
   EXPECT_EQ(12, designPoint0[1]);
   EXPECT_EQ(21, designPoint1[0]);
   EXPECT_EQ(22, designPoint1[1]);

   QVector<bool> completed = manager.completed();
   EXPECT_TRUE(completed[0]);
   EXPECT_FALSE(completed[1]);
}

TEST( ManualDesignPointManagerTest, isCompleted )
{
   casaWizard::ua::ManualDesignPointManager manager;
   manager.addDesignPoint();
   manager.addDesignPoint();

   manager.setCompleted(0,true);
   EXPECT_TRUE(manager.isCompleted(0));
}

TEST( ManualDesignPointManagerTest, setAllToNotCompleteAndRemoveHiddenPoints )
{
   casaWizard::ua::ManualDesignPointManager manager;
   manager.addDesignPoint();
   manager.addDesignPoint();

   manager.setCompleted(0,true);
   manager.setAllIncomplete();
   QVector<bool> completed = manager.completed();
   QVector<bool> completedExpected{false,false};
   EXPECT_EQ(completed,completedExpected);
}

TEST( ManualDesignPointManagerTest, clear )
{
   casaWizard::ua::ManualDesignPointManager manager;
   manager.addInfluentialParameter(2);
   manager.addDesignPoint({1,2});
   manager.addDesignPoint({3,4});
   manager.clear();
   EXPECT_EQ(manager.parameters().size(),0);
   EXPECT_EQ(manager.numberOfVisiblePoints(),0);
   EXPECT_EQ(manager.numberOfParameters(),0);
}
