#include <gtest/gtest.h>
#include "model/ManualDesignPoint.h"

#include <QStringList>

using namespace casaWizard;
using namespace ua;

TEST( ManualDesignPointTest, testDefaultConstruct )
{
   ManualDesignPoint manualDesignPoint;
   EXPECT_FALSE(manualDesignPoint.hidden());
   EXPECT_EQ(manualDesignPoint.simulationState(), ManualDesignPoint::SimulationState::NotRun);
}

TEST( ManualDesignPointTest, testConstructValues )
{
   ManualDesignPoint manualDesignPoint({1,2,3});
   EXPECT_FALSE(manualDesignPoint.hidden());
   EXPECT_EQ(manualDesignPoint.simulationState(), ManualDesignPoint::SimulationState::NotRun);
   QVector<double> paramsExpected{1,2,3};
   EXPECT_EQ(manualDesignPoint.influentialParameterValues(),paramsExpected);
}

TEST( ManualDesignPointTest, testAddParameters )
{
   ManualDesignPoint manualDesignPoint;
   manualDesignPoint.addInfluentialParameter(1);
   manualDesignPoint.addInfluentialParameter(-1);
   QVector<double> paramsExpected{1,-1};
   EXPECT_EQ(manualDesignPoint.influentialParameterValues(),paramsExpected);
}

TEST( ManualDesignPointTest, testHide )
{
   ManualDesignPoint manualDesignPoint;
   manualDesignPoint.hide();
   EXPECT_TRUE(manualDesignPoint.hidden());
}

TEST( ManualDesignPointTest, testRemoveParameter )
{
   ManualDesignPoint manualDesignPoint({1,2,3});
   manualDesignPoint.removeParameter(0);
   QVector<double> paramsExpected{2,3};
   EXPECT_EQ(manualDesignPoint.influentialParameterValues(),paramsExpected);
   manualDesignPoint.removeParameter(2); //Invalid index, no parameters removed
   EXPECT_EQ(manualDesignPoint.influentialParameterValues(),paramsExpected);
   manualDesignPoint.removeParameter(-1); //Invalid index, no parameters removed
   EXPECT_EQ(manualDesignPoint.influentialParameterValues(),paramsExpected);
}

TEST( ManualDesignPointTest, testSetParameterValue )
{
   ManualDesignPoint manualDesignPoint({1,2,3});
   manualDesignPoint.setParameterValue(1, 10);
   QVector<double> paramsExpected{1,10,3};
   EXPECT_EQ(manualDesignPoint.influentialParameterValues(),paramsExpected);
   manualDesignPoint.setParameterValue(3, 10);//Invalid index, no value changed
   EXPECT_EQ(manualDesignPoint.influentialParameterValues(),paramsExpected);
   manualDesignPoint.setParameterValue(-1, 10);//Invalid index, no value changed
   EXPECT_EQ(manualDesignPoint.influentialParameterValues(),paramsExpected);
}

TEST( ManualDesignPointTest, testSetSimulationState)
{
   ManualDesignPoint manualDesignPoint;

   //Set simulation state by state:
   manualDesignPoint.setSimulationState(ManualDesignPoint::SimulationState::Failed);
   EXPECT_EQ(manualDesignPoint.simulationState(),ManualDesignPoint::SimulationState::Failed);

   //Set simulation state by bool:
   manualDesignPoint.setSimulationState(true);
   EXPECT_EQ(manualDesignPoint.simulationState(),ManualDesignPoint::SimulationState::Completed);
}

TEST( ManualDesignPointTest, testReadWrite)
{
   ManualDesignPoint manualDesignPoint({1,2,3});
   manualDesignPoint.setSimulationState(ManualDesignPoint::SimulationState::Failed);
   manualDesignPoint.hide();
   QStringList writeString = manualDesignPoint.write();
   int version = manualDesignPoint.version();

   ManualDesignPoint readPoint = ManualDesignPoint::read(version,writeString);

   EXPECT_EQ(manualDesignPoint.influentialParameterValues(),readPoint.influentialParameterValues());
   EXPECT_EQ(manualDesignPoint.hidden(),readPoint.hidden());
   EXPECT_EQ(manualDesignPoint.simulationState(),readPoint.simulationState());
}
